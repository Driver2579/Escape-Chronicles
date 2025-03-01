// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeChronicles/Public/Characters/EscapeChroniclesCharacter.h"

#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/ArrowComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "DefaultMovementSet/NavMoverComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesCharacter::AEscapeChroniclesCharacter()
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("Capsule Component");
	RootComponent = CapsuleComponent;

	CapsuleComponent->InitCapsuleSize(42.f, 96.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(CapsuleComponent);

	MeshComponent->AlwaysLoadOnClient = true;
	MeshComponent->AlwaysLoadOnServer = true;
	MeshComponent->bOwnerNoSee = false;
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	MeshComponent->bCastDynamicShadow = true;
	MeshComponent->bAffectDynamicIndirectLighting = true;
	MeshComponent->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetCanEverAffectNavigation(false);

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));

	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = TEXT("Characters");
		ArrowComponent->SpriteInfo.DisplayName = NSLOCTEXT("SpriteCategory", "Characters", "Characters");
		ArrowComponent->SetupAttachment(CapsuleComponent);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoomComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoomComponent->SetupAttachment(RootComponent);

	// The camera follows at this distance behind the character
	CameraBoomComponent->TargetArmLength = 400.0f;

	// Rotate the arm based on the controller
	CameraBoomComponent->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCameraComponent->SetupAttachment(CameraBoomComponent, USpringArmComponent::SocketName);

	// The camera does not rotate relative to SpringArmComponent
	FollowCameraComponent->bUsePawnControlRotation = false;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CharacterMoverComponent = CreateDefaultSubobject<UCharacterMoverComponent>(TEXT("Mover Component"));

	if (USceneComponent* UpdatedComponent = CharacterMoverComponent->GetUpdatedComponent())
	{
		UpdatedComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
	}

	// Disable Actor-level movement replication, since our Mover component will handle it
	SetReplicatingMovement(false);
}

void AEscapeChroniclesCharacter::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITORONLY_DATA
	if (ArrowComponent)
	{
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif
}

void AEscapeChroniclesCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	NavMoverComponent = FindComponentByClass<UNavMoverComponent>();
}

void AEscapeChroniclesCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	// InitAbilityActorInfo on server and client
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
	}
}

FVector AEscapeChroniclesCharacter::GetNavAgentLocation() const
{
	// The code below was copied from the MoverExamplesCharacter::GetNavAgentLocation method

	FVector AgentLocation = FNavigationSystem::InvalidLocation;

	const USceneComponent* UpdatedComponent =
		CharacterMoverComponent ? CharacterMoverComponent->GetUpdatedComponent() : nullptr;

	if (NavMoverComponent)
	{
		AgentLocation = NavMoverComponent->GetFeetLocation();
	}

	if (FNavigationSystem::IsValidLocation(AgentLocation) == false && UpdatedComponent != nullptr)
	{
		AgentLocation = UpdatedComponent->GetComponentLocation() -
			FVector(0, 0, UpdatedComponent->Bounds.BoxExtent.Z);
	}

	return AgentLocation;
}

void AEscapeChroniclesCharacter::UpdateNavigationRelevance()
{
	// The code below was copied from the MoverExamplesCharacter::UpdateNavigationRelevance method

	if (CharacterMoverComponent)
	{
		if (USceneComponent* UpdatedComponent = CharacterMoverComponent->GetUpdatedComponent())
		{
			UpdatedComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
		}
	}
}

void AEscapeChroniclesCharacter::AddMovementInput(const FVector WorldDirection, const float ScaleValue,
	const bool bForce)
{
	if (bForce || !IsMoveInputIgnored())
	{
		ControlInputVector = WorldDirection * ScaleValue;
	}
}

FVector AEscapeChroniclesCharacter::ConsumeMovementInputVector()
{
	return Internal_ConsumeMovementInputVector();
}

void AEscapeChroniclesCharacter::ProduceInput_Implementation(int32 SimTimeMs,
	FMoverInputCmdContext& InputCmdResult)
{
	// The code below was copied from the MoverExamplesCharacter::OnProduceInput method with some modifications

	FCharacterDefaultInputs& CharacterInputs = InputCmdResult.InputCollection.FindOrAddMutableDataByType<
		FCharacterDefaultInputs>();

	if (!Controller)
	{
		if (GetLocalRole() == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
		{
			static const FCharacterDefaultInputs DoNothingInput;

			/**
			 * If we get here, that means this pawn is not currently possessed, and we're choosing to provide default
			 * do-nothing input
			 */
			CharacterInputs = DoNothingInput;
		}

		/**
		 * We don't have a local controller, so we can't run the code below. This is ok. Simulated proxies will just use
		 * previous input when extrapolating.
		 */
		return;
	}

	CharacterInputs.ControlRotation = GetControlRotation();

	bool bRequestedNavMovement = false;

	if (NavMoverComponent)
	{
		bRequestedNavMovement = NavMoverComponent->bRequestedNavMovement;

		if (bRequestedNavMovement)
		{
			ControlInputVector = NavMoverComponent->CachedNavMoveInputIntent;
			CachedMoveInputVelocity = NavMoverComponent->CachedNavMoveInputVelocity;

			NavMoverComponent->bRequestedNavMovement = false;
			NavMoverComponent->CachedNavMoveInputIntent = FVector::ZeroVector;
			NavMoverComponent->CachedNavMoveInputVelocity = FVector::ZeroVector;
		}
	}

	// Favor velocity input
	const bool bUsingInputIntentForMove = CachedMoveInputVelocity.IsZero();

	if (bUsingInputIntentForMove)
	{
		FRotator Rotator = CharacterInputs.ControlRotation;
		FVector FinalDirectionalIntent;

		if (CharacterMoverComponent)
		{
			if (CharacterMoverComponent->IsOnGround() || CharacterMoverComponent->IsFalling())
			{
				const FVector RotationProjectedOntoUpDirection = FVector::VectorPlaneProject(Rotator.Vector(),
					CharacterMoverComponent->GetUpDirection()).GetSafeNormal();

				Rotator = RotationProjectedOntoUpDirection.Rotation();
			}

			FinalDirectionalIntent = Rotator.RotateVector(ControlInputVector);
		}

		CharacterInputs.SetMoveInput(EMoveInputType::DirectionalIntent, FinalDirectionalIntent);
	}
	else
	{
		CharacterInputs.SetMoveInput(EMoveInputType::Velocity, CachedMoveInputVelocity);
	}

	/**
	 * Normally, cached input is cleared by OnMoveCompleted input event, but that won't be called if movement came from
	 * nav movement
	 */
	if (bRequestedNavMovement)
	{
		ControlInputVector = FVector::ZeroVector;
		CachedMoveInputVelocity = FVector::ZeroVector;
	}

	constexpr float RotationMagMin(1e-3);

	const bool bHasAffirmativeMoveInput = CharacterInputs.GetMoveInput().Size() >= RotationMagMin;

	// Figure out the intended orientation
	CharacterInputs.OrientationIntent = FVector::ZeroVector;

	if (bHasAffirmativeMoveInput)
	{
		if (bOrientRotationToMovement)
		{
			// Set the intent to the actor's movement direction
			CharacterInputs.OrientationIntent = CharacterInputs.GetMoveInput().GetSafeNormal();
		}
		else
		{
			// Set intent to the control rotation - often a player's camera rotation
			CharacterInputs.OrientationIntent = CharacterInputs.ControlRotation.Vector().GetSafeNormal();
		}

		LastAffirmativeMoveInput = CharacterInputs.GetMoveInput();
	}
	else if (bMaintainLastInputOrientation)
	{
		// There is no movement intent, so use the last-known affirmative move input
		CharacterInputs.OrientationIntent = LastAffirmativeMoveInput;
	}

	if (bShouldRemainVertical)
	{
		// Canceling out any z intent if the actor is supposed to remain vertical
		CharacterInputs.OrientationIntent = CharacterInputs.OrientationIntent.GetSafeNormal2D();
	}

	CharacterInputs.bIsJumpPressed = bIsJumpPressed;
	CharacterInputs.bIsJumpJustPressed = bIsJumpJustPressed;

	CharacterInputs.SuggestedMovementMode = NAME_None;

	// Convert inputs to be relative to the current movement base (depending on options and state)
	CharacterInputs.bUsingMovementBase = false;

	if (bUseBaseRelativeMovement && CharacterMoverComponent)
	{
		if (UPrimitiveComponent* MovementBase = CharacterMoverComponent->GetMovementBase())
		{
			const FName MovementBaseBoneName = CharacterMoverComponent->GetMovementBaseBoneName();

			FVector RelativeMoveInput, RelativeOrientDir;

			UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName,
				CharacterInputs.GetMoveInput(), RelativeMoveInput);

			UBasedMovementUtils::TransformWorldDirectionToBased(MovementBase, MovementBaseBoneName,
				CharacterInputs.OrientationIntent, RelativeOrientDir);

			CharacterInputs.SetMoveInput(CharacterInputs.GetMoveInputType(), RelativeMoveInput);
			CharacterInputs.OrientationIntent = RelativeOrientDir;

			CharacterInputs.bUsingMovementBase = true;
			CharacterInputs.MovementBase = MovementBase;
			CharacterInputs.MovementBaseBoneName = MovementBaseBoneName;
		}
	}

	/**
	 * Clear/consume temporal movement inputs. We are not consuming others in the event that the game world is
	 * ticking at a lower rate than the Mover simulation. In that case, we want most input to carry over between
	 * simulation frames.
	 */
	bIsJumpJustPressed = false;
}

void AEscapeChroniclesCharacter::Look(const FVector2D& LookAxisVector)
{
	// Add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AEscapeChroniclesCharacter::Move(FVector MovementVector)
{
	// Make sure the movement vector is clamped between -1 and 1
	MovementVector.X = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
	MovementVector.Y = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);
	MovementVector.Z = FMath::Clamp(MovementVector.Z, -1.0f, 1.0f);

	// Set the movement vector
	AddMovementInput(MovementVector);
}

void AEscapeChroniclesCharacter::StopMoving()
{
	ConsumeMovementInputVector();
}

void AEscapeChroniclesCharacter::Jump()
{
	// The code below was copied from the MoverExamplesCharacter::OnJumpStarted method

	bIsJumpJustPressed = !bIsJumpPressed;
	bIsJumpPressed = true;
}

void AEscapeChroniclesCharacter::StopJumping()
{
	// The code below was copied from the MoverExamplesCharacter::OnJumpReleased method

	bIsJumpPressed = false;
	bIsJumpJustPressed = false;
}

UAbilitySystemComponent* AEscapeChroniclesCharacter::GetAbilitySystemComponent() const
{
	const AEscapeChroniclesPlayerState* EscapeChroniclesPlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		GetPlayerState(), ECastCheckedType::NullAllowed);

	return IsValid(EscapeChroniclesPlayerState) ? EscapeChroniclesPlayerState->GetAbilitySystemComponent() : nullptr;
}

UEscapeChroniclesAbilitySystemComponent* AEscapeChroniclesCharacter::GetEscapeChroniclesAbilitySystemComponent() const
{
	const AEscapeChroniclesPlayerState* EscapeChroniclesPlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		GetPlayerState(), ECastCheckedType::NullAllowed);

	return IsValid(EscapeChroniclesPlayerState) ?
		EscapeChroniclesPlayerState->GetEscapeChroniclesAbilitySystemComponent() : nullptr;
}