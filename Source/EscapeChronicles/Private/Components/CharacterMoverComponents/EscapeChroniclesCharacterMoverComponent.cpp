﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"

#include "EscapeChroniclesGameplayTags.h"
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

void UEscapeChroniclesCharacterMoverComponent::InitializeComponent()
{
	Super::InitializeComponent();

#if DO_CHECK
	check(IsValid(GetOwner()));
	check(GetOwner()->IsA<APawn>());
#endif

	const APawn* OwningPawn = CastChecked<APawn>(GetOwner());

	// Remember which rotation settings were set by default
	bDefaultUseControllerRotationPitch = OwningPawn->bUseControllerRotationPitch;
	bDefaultUseControllerRotationYaw = OwningPawn->bUseControllerRotationYaw;
	bUseControllerRotationRoll = OwningPawn->bUseControllerRotationRoll;
}

void UEscapeChroniclesCharacterMoverComponent::DisableMovement()
{
	QueueNextMode(NullModeName);

#if DO_CHECK
	check(IsValid(GetOwner()));
	check(GetOwner()->IsA<APawn>());
#endif

	APawn* OwningPawn = CastChecked<APawn>(GetOwner());

	// Disable any rotation on the pawn
	OwningPawn->bUseControllerRotationPitch = false;
	OwningPawn->bUseControllerRotationYaw = false;
	OwningPawn->bUseControllerRotationRoll = false;

	// TODO: It is also necessary that when the movement is turned off, the rotation does not work too! 

	// TODO: Find out how to disable Mover completely and do it here
	//CachedLastSyncState.SyncStateCollection.Empty();
	//CachedLastSyncState.Reset();
	//LastMoverDefaultSyncState = nullptr;
}

void UEscapeChroniclesCharacterMoverComponent::SetDefaultMovementMode()
{
	QueueNextMode(StartingMovementMode);

#if DO_CHECK
	check(IsValid(GetOwner()));
	check(GetOwner()->IsA<APawn>());
#endif

	APawn* OwningPawn = CastChecked<APawn>(GetOwner());

	// Get the default rotation settings back to the pawn
	OwningPawn->bUseControllerRotationPitch = bDefaultUseControllerRotationPitch;
	OwningPawn->bUseControllerRotationYaw = bDefaultUseControllerRotationYaw;
	OwningPawn->bUseControllerRotationRoll = bUseControllerRotationRoll;
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