// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"

#include "EscapeChroniclesGameplayTags.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Enums/Mover/GroundSpeedMode.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Mover/MovementModifiers/GroundSpeedModeModifier.h"
#include "Mover/MovementSettings/GroundSpeedModeSettings.h"

const FName UEscapeChroniclesCharacterMoverComponent::NullModeName(TEXT("Null"));

#if WITH_EDITOR
void UEscapeChroniclesCharacterMoverComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UCommonLegacyMovementSettings* CommonLegacyMovementSettings = FindSharedSettings_Mutable<
		UCommonLegacyMovementSettings>();

	const UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings<UGroundSpeedModeSettings>();

	/**
	 * Override MaxSpeed in CommonLegacyMovementSettings if GroundSpeedModeSettings exist to avoid confusion since
	 * MaxSpeed is always overriden by GroundSpeedModeModifier in runtime if it exists.
	 */
	if (IsValid(CommonLegacyMovementSettings) && IsValid(GroundSpeedModeSettings))
	{
		CommonLegacyMovementSettings->MaxSpeed = GroundSpeedModeSettings->GetDefaultMaxSpeed();
	}
}
#endif

void UEscapeChroniclesCharacterMoverComponent::DisableMovement()
{
	QueueNextMode(NullModeName);

	// TODO: It is also necessary that when the movement is turned off, the rotation does not work too! 

	// TODO: Find out how to disable Mover completely and do it here
	//CachedLastSyncState.SyncStateCollection.Empty();
	//CachedLastSyncState.Reset();
	//LastMoverDefaultSyncState = nullptr;
}

void UEscapeChroniclesCharacterMoverComponent::SetDefaultMovementMode()
{
	QueueNextMode(StartingMovementMode);
}

bool UEscapeChroniclesCharacterMoverComponent::DoesMaxSpeedWantToBeOverriden() const
{
	// We use cons_cast here because CanCrouch should be const, BUT EPIC GAMES FOR SOME FUCKING REASON MADE IT NON-CONST
	return IsCrouching() || (bWantsToCrouch && const_cast<ThisClass*>(this)->CanCrouch());
}

bool UEscapeChroniclesCharacterMoverComponent::IsWalkGroundSpeedModeActive() const
{
	return HasGameplayTag(EscapeChroniclesGameplayTags::Mover_IsWalkGroundSpeedModeActive, true);
}

bool UEscapeChroniclesCharacterMoverComponent::IsJogGroundSpeedModeActive() const
{
	return HasGameplayTag(EscapeChroniclesGameplayTags::Mover_IsJogGroundSpeedModeActive, true);
}

bool UEscapeChroniclesCharacterMoverComponent::IsRunGroundSpeedModeActive() const
{
	return HasGameplayTag(EscapeChroniclesGameplayTags::Mover_IsRunGroundSpeedModeActive, true);
}

void UEscapeChroniclesCharacterMoverComponent::SetGroundSpeedMode(const EGroundSpeedMode NewGroundSpeedMode) const
{
#if DO_ENSURE
	ensureAlwaysMsgf(NewGroundSpeedMode < EGroundSpeedMode::NumberOfModes,
		TEXT("Invalid NewGroundSpeedMode was passed!"));
#endif

	UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings_Mutable<UGroundSpeedModeSettings>();

	if (ensureAlways(IsValid(GroundSpeedModeSettings)))
	{
		GroundSpeedModeSettings->GroundSpeedMode = NewGroundSpeedMode;
	}
}

void UEscapeChroniclesCharacterMoverComponent::ResetGroundSpeedMode() const
{
	UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings_Mutable<UGroundSpeedModeSettings>();

	if (ensureAlways(IsValid(GroundSpeedModeSettings)))
	{
		GroundSpeedModeSettings->GroundSpeedMode = GroundSpeedModeSettings->GetDefaultGroundSpeedMode();
	}
}

float UEscapeChroniclesCharacterMoverComponent::GetMaxSpeed() const
{
	const UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings_Mutable<UGroundSpeedModeSettings>();

	if (ensureAlways(IsValid(GroundSpeedModeSettings)))
	{
		return GroundSpeedModeSettings->GetSelectedMaxSpeed();
	}

	return -1.f;
}

void UEscapeChroniclesCharacterMoverComponent::OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep,
	const FMoverInputCmdContext& InputCmd)
{
	Super::OnMoverPreSimulationTick(TimeStep, InputCmd);
	
	const UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings<UGroundSpeedModeSettings>();

	if (IsValid(GroundSpeedModeSettings))
	{
		/**
		 * Add GroundSpeedModeModifier only if GroundSpeedModeSettings exist.
		 *
		 * Note: We could've do that in BeginPlay but in this case this modifier's handle gets invalidated on clients at
		 * some point, which causes bugs with other modifiers. So unfortunately we have to add it here every tick for
		 * its handle to be regenerated in case it gets invalidated.
		 */
		QueueMovementModifier(MakeShared<FGroundSpeedModeModifier>());

		// Check if the GroundSpeedMode has changed
		if (GroundSpeedModeSettings->GroundSpeedMode != LastGroundSpeedMode)
		{
			OnGroundSpeedModeChanged.Broadcast(LastGroundSpeedMode, GroundSpeedModeSettings->GroundSpeedMode);

			LastGroundSpeedMode = GroundSpeedModeSettings->GroundSpeedMode;
		}
	}
}

void UEscapeChroniclesCharacterMoverComponent::OnHandleImpact(const FMoverOnImpactParams& ImpactParams)
{
	Super::OnHandleImpact(ImpactParams);

	const AEscapeChroniclesCharacter* CharacterOwner = GetOwner<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(CharacterOwner)))
	{
		return;
	}
	
	if (bEnablePhysicsInteraction)
	{
		const FVector FallingAcceleration =
			-CharacterOwner->GetGravityDirection() * CharacterOwner->GetGravityTransform().Z;

		const FVector ForceAccel =
			ImpactParams.AttemptedMoveDelta + (IsFalling() ? FallingAcceleration : FVector::ZeroVector);
		
		ApplyImpactPhysicsForces(ImpactParams.HitResult, ForceAccel, GetVelocity());
	}
}

void UEscapeChroniclesCharacterMoverComponent::ApplyImpactPhysicsForces(const FHitResult& Impact,
	const FVector& ImpactAcceleration, const FVector& ImpactVelocity)
{
	if (bEnablePhysicsInteraction && Impact.bBlockingHit )
	{
		if (UPrimitiveComponent* ImpactComponent = Impact.GetComponent())
		{
			FVector ForcePoint = Impact.ImpactPoint;
			float BodyMass = 1.0f; // set to 1 as this is used as a multiplier

			bool bCanBePushed = false;
			FBodyInstance* BI = ImpactComponent->GetBodyInstance(Impact.BoneName);
			if(BI != nullptr && BI->IsInstanceSimulatingPhysics())
			{
				BodyMass = FMath::Max(BI->GetBodyMass(), 1.0f);

				bCanBePushed = true;
			}

			if (bCanBePushed)
			{
				FVector Force = Impact.ImpactNormal * -1.0f;

				float PushForceModificator = 1.0f;

				const FVector ComponentVelocity = ImpactComponent->GetPhysicsLinearVelocity();
				const FVector VirtualVelocity =
					ImpactAcceleration.IsZero() ? ImpactVelocity : ImpactAcceleration.GetSafeNormal() * GetMaxSpeed();

				if (bScalePushForceToVelocity && !ComponentVelocity.IsNearlyZero())
				{
					float Dot = 0.0f;
					
					Dot = ComponentVelocity | VirtualVelocity;

					if (Dot > 0.0f && Dot < 1.0f)
					{
						PushForceModificator *= Dot;
					}
				}

				if (bPushForceScaledToMass)
				{
					PushForceModificator *= BodyMass;
				}
				
				Force *= PushForceModificator;

				if (ComponentVelocity.IsNearlyZero(1.0f))
				{
					Force *= InitialPushForceFactor;
					ImpactComponent->AddImpulseAtLocation(Force, ForcePoint, Impact.BoneName);
				}
				else
				{
					Force *= PushForceFactor;
					ImpactComponent->AddForceAtLocation(Force, ForcePoint, Impact.BoneName);
				}
			}
		}
	}
}
