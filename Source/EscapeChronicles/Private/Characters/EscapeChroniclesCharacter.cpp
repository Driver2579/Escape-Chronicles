// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/Characters/EscapeChroniclesCharacter.h"

#include "AbilitySystemComponent.h"
#include "EscapeChroniclesGameplayTags.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Common/Enums/Mover/GroundSpeedMode.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ActorComponents/CarryCharacterComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"
#include "DefaultMovementSet/NavMoverComponent.h"
#include "Engine/AssetManager.h"
#include "Mover/Inputs/EscapeChroniclesCharacterExtendedDefaultInputs.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AEscapeChroniclesCharacter::AEscapeChroniclesCharacter()
	: DesiredGroundSpeedModeOverride(EGroundSpeedMode::None)
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
	MeshComponent->SetUsingAbsoluteRotation(true);

	CarryCharacterComponent = CreateDefaultSubobject<UCarryCharacterComponent>(TEXT("Carry Character Component"));

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
	CameraBoomComponent->SetupAttachment(MeshComponent);

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

	CharacterMoverComponent = CreateDefaultSubobject<UEscapeChroniclesCharacterMoverComponent>(TEXT("Mover Component"));

	if (USceneComponent* UpdatedComponent = CharacterMoverComponent->GetUpdatedComponent())
	{
		UpdatedComponent->SetCanEverAffectNavigation(bCanAffectNavigationGeneration);
	}

	// Disable Actor-level movement replication, since our Mover component will handle it
	SetReplicatingMovement(false);

	// === Interaction ===

	InteractionManagerComponent = CreateDefaultSubobject<UInteractionManagerComponent>(
		TEXT("Interaction Manager Component"));

	InteractionManagerComponent->SetupAttachment(RootComponent);

	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("Interaction Zone"));
	InteractionZone->SetupAttachment(InteractionManagerComponent);

	// === Inventory ===

	InventoryManagerComponent = CreateDefaultSubobject<UInventoryManagerComponent>(TEXT("Inventory Manager Component"));
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

	// NavMoverComponent is optionally added to the character blueprint to support AI navigation
	NavMoverComponent = FindComponentByClass<UNavMoverComponent>();

	CharacterMoverComponent->OnPostMovement.AddDynamic(this, &ThisClass::OnMoverPostMovement);
	CharacterMoverComponent->OnPreSimulationTick.AddDynamic(this, &ThisClass::OnMoverPreSimulationTick);

	CharacterMoverComponent->OnMovementModeChanged.AddDynamic(this, &ThisClass::OnMovementModeChanged);
	CharacterMoverComponent->OnStanceChanged.AddDynamic(this, &ThisClass::OnStanceChanged);
	CharacterMoverComponent->OnGroundSpeedModeChanged.AddUObject(this, &ThisClass::OnGroundSpeedModeChanged);

	DefaultMeshCollisionProfileName = MeshComponent->GetCollisionProfileName();
	DefaultCapsuleCollisionProfileName = CapsuleComponent->GetCollisionProfileName();

	InitialMeshTransform = MeshComponent->GetRelativeTransform();
	InitialMeshAttachParent = MeshComponent->GetAttachParent();
}

void AEscapeChroniclesCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasAnyMatchingGameplayTags(MeshControllingStateTags))
	{
		return;
	}

	const FRotator MeshRotation = MeshComponent->GetComponentRotation() - InitialMeshTransform.Rotator();
	const FRotator ActorRotation = GetActorRotation();

	ActorAndViewDelta = MeshRotation - ActorRotation;
	ActorAndViewDelta.Normalize();

	const float AbsoluteYawDelta = FMath::Abs(ActorAndViewDelta.Yaw);

	// Check if we have to rotate the actor
	if (!bTurning && AbsoluteYawDelta <= AngleToStartTurning && CharacterMoverComponent->GetVelocity().Length() == 0)
	{
		return;
	}

	// Check or end the current turn
	bTurning = AbsoluteYawDelta > AngleToStopTurning;

	// === Rotate actor ===

	FRotator NewMeshRotation = MeshRotation;

	NewMeshRotation.Yaw = ActorRotation.Yaw + InitialMeshTransform.Rotator().Yaw -
		FMath::FInterpTo(-ActorAndViewDelta.Yaw, 0, DeltaSeconds, TurningInterpSpeed);

	MeshComponent->SetRelativeRotation(NewMeshRotation);
}

void AEscapeChroniclesCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	// InitAbilityActorInfo on server and client
	AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);

	// Apply all active gameplay tags from the CharacterMoverComponent to the AbilitySystemComponent
	SyncCharacterMoverComponentTagsWithAbilitySystem();

	AbilitySystemComponent->RegisterGenericGameplayTagEvent().AddUObject(this, &ThisClass::UpdateMeshControllingState);

	// === Subscribe to changes in the health attribute ===

	const UVitalAttributeSet* VitalAttributeSet = AbilitySystemComponent->GetSet<UVitalAttributeSet>();

	if (IsValid(VitalAttributeSet))
	{
		FOnGameplayAttributeValueChange& OnHealthAttributeValueChangeDelegate =
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate( VitalAttributeSet->GetHealthAttribute());

		OnHealthAttributeValueChangeDelegate.AddUObject(this, &ThisClass::OnHealthChanged);
	}

	UpdateFaintedState();
}

FVector AEscapeChroniclesCharacter::GetNavAgentLocation() const
{
	// === The code below was copied from the MoverExamplesCharacter::GetNavAgentLocation method ===

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
	// === The code below was copied from the MoverExamplesCharacter::UpdateNavigationRelevance method ===

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
	// === The code below was copied from the MoverExamplesCharacter::OnProduceInput method with some modifications ===

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
		bRequestedNavMovement = NavMoverComponent->ConsumeNavMovementData(ControlInputVector,
			CachedMoveInputVelocity);
	}

	// Favor velocity input
	const bool bUsingInputIntentForMove = CachedMoveInputVelocity.IsZero();

	if (bUsingInputIntentForMove)
	{
		const FVector FinalDirectionalIntent = CharacterInputs.ControlRotation.RotateVector(ControlInputVector);
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

	static constexpr float RotationMagMin(1e-3);

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
	else if (bUseControllerRotationPitch || bUseControllerRotationYaw || bUseControllerRotationRoll)
	{
		// Set intent to the control rotation - often a player's camera rotation
		CharacterInputs.OrientationIntent = CharacterInputs.ControlRotation.Vector().GetSafeNormal();
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

	if (bUseBaseRelativeMovement)
	{
		UPrimitiveComponent* MovementBase = CharacterMoverComponent->GetMovementBase();

		if (IsValid(MovementBase))
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

	// === The code below is completely custom and wasn't copied from anywhere ===

	FEscapeChroniclesCharacterExtendedDefaultInputs& ExtendedCharacterInputs =
		InputCmdResult.InputCollection.FindOrAddMutableDataByType<FEscapeChroniclesCharacterExtendedDefaultInputs>();

	ExtendedCharacterInputs.bWantsToBeCrouched = bWantsToBeCrouched;
	ExtendedCharacterInputs.DesiredGroundSpeedModeOverride = DesiredGroundSpeedModeOverride;
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void AEscapeChroniclesCharacter::OnMoverPostMovement(const FMoverTimeStep& TimeStep, FMoverSyncState& SyncState,
	FMoverAuxStateContext& AuxState)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	const FMoverDefaultSyncState* DefaultSyncState = SyncState.SyncStateCollection.FindDataByType<
		FMoverDefaultSyncState>();

#if DO_CHECK
	check(DefaultSyncState);
#endif

	// Check if the character is moving or not (velocity isn't zero)
	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Moving,
		!DefaultSyncState->GetVelocity_BaseSpace().IsNearlyZero() ? 1 : 0);
}

void AEscapeChroniclesCharacter::OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep,
	const FMoverInputCmdContext& InputCmd)
{
	const FEscapeChroniclesCharacterExtendedDefaultInputs* ExtendedCharacterInputs =
		InputCmd.InputCollection.FindDataByType<FEscapeChroniclesCharacterExtendedDefaultInputs>();

	if (!ExtendedCharacterInputs)
	{
		return;
	}

	if (ExtendedCharacterInputs->bWantsToBeCrouched)
	{
		CharacterMoverComponent->Crouch();
	}
	else
	{
		CharacterMoverComponent->UnCrouch();
	}

	if (ExtendedCharacterInputs->DesiredGroundSpeedModeOverride != EGroundSpeedMode::None)
	{
		CharacterMoverComponent->SetGroundSpeedMode(ExtendedCharacterInputs->DesiredGroundSpeedModeOverride);
	}
	else
	{
		CharacterMoverComponent->ResetGroundSpeedMode();
	}
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
	// === The code below was copied from the MoverExamplesCharacter::OnJumpStarted method ===

	bIsJumpJustPressed = !bIsJumpPressed;
	bIsJumpPressed = true;
}

void AEscapeChroniclesCharacter::StopJumping()
{
	// === The code below was copied from the MoverExamplesCharacter::OnJumpReleased method ===

	bIsJumpPressed = false;
	bIsJumpJustPressed = false;
}

void AEscapeChroniclesCharacter::Crouch()
{
	bWantsToBeCrouched = true;
}

void AEscapeChroniclesCharacter::UnCrouch()
{
	bWantsToBeCrouched = false;
}

void AEscapeChroniclesCharacter::OverrideGroundSpeedMode(const EGroundSpeedMode GroundSpeedModeOverride)
{
#if DO_ENSURE
	ensureAlwaysMsgf(GroundSpeedModeOverride < EGroundSpeedMode::NumberOfModes,
		TEXT("Invalid GroundSpeedModeOverride was passed!"));
#endif

	DesiredGroundSpeedModeOverride = GroundSpeedModeOverride;
}

void AEscapeChroniclesCharacter::ResetGroundSpeedMode(const EGroundSpeedMode GroundSpeedModeOverrideToReset)
{
	// Don't reset the override if the one we want to reset isn't the one that is currently set
	if (GroundSpeedModeOverrideToReset == DesiredGroundSpeedModeOverride)
	{
		DesiredGroundSpeedModeOverride = EGroundSpeedMode::None;
	}
}

void AEscapeChroniclesCharacter::OnMovementModeChanged(const FName& PreviousMovementModeName,
	const FName& NewMovementModeName)
{
	/**
	 * Even though the movement mode is changed, we need to wait for the next tick because gameplay tags in the
	 * CharacterMoverComponent are updated only in the next tick.
	 */
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this,
		&ThisClass::SyncMovementModesTagsWithAbilitySystem));
}

void AEscapeChroniclesCharacter::OnStanceChanged(const EStanceMode OldStance, const EStanceMode NewStance)
{
	SyncStancesTagsWithAbilitySystem(OldStance, NewStance);
}

void AEscapeChroniclesCharacter::OnGroundSpeedModeChanged(const EGroundSpeedMode OldGroundSpeedMode,
	const EGroundSpeedMode NewGroundSpeedMode)
{
	SyncGroundSpeedModeTagsWithAbilitySystem(OldGroundSpeedMode, NewGroundSpeedMode);
}

void AEscapeChroniclesCharacter::SyncCharacterMoverComponentTagsWithAbilitySystem() const
{
	SyncMovementModesTagsWithAbilitySystem();
	SyncStancesTagsWithAbilitySystem();
	SyncGroundSpeedModeTagsWithAbilitySystem();
}

void AEscapeChroniclesCharacter::SyncMovementModesTagsWithAbilitySystem() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Falling,
		CharacterMoverComponent->IsFalling() ? 1 : 0);

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_InAir,
		CharacterMoverComponent->IsAirborne() ? 1 : 0);

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_NavWalking,
		CharacterMoverComponent->HasGameplayTag(Mover_IsNavWalking, true) ? 1 : 0);

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_OnGround,
		CharacterMoverComponent->IsOnGround() ? 1 : 0);
}

void AEscapeChroniclesCharacter::SyncStancesTagsWithAbilitySystem() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Crouching,
		CharacterMoverComponent->IsCrouching() ? 1 : 0);
}

void AEscapeChroniclesCharacter::SyncGroundSpeedModeTagsWithAbilitySystem() const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Walking,
		CharacterMoverComponent->IsWalkGroundSpeedModeActive() ? 1 : 0);

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Jogging,
		CharacterMoverComponent->IsJogGroundSpeedModeActive() ? 1 : 0);

	AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Running,
		CharacterMoverComponent->IsRunGroundSpeedModeActive() ? 1 : 0);
}

void AEscapeChroniclesCharacter::SyncStancesTagsWithAbilitySystem(const EStanceMode OldStance,
	const EStanceMode NewStance) const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	if (OldStance == EStanceMode::Crouch)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Crouching,
			0);
	}

	if (NewStance == EStanceMode::Crouch)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Crouching,
			1);
	}
}

void AEscapeChroniclesCharacter::SyncGroundSpeedModeTagsWithAbilitySystem(const EGroundSpeedMode OldGroundSpeedMode,
	const EGroundSpeedMode NewGroundSpeedMode) const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	if (OldGroundSpeedMode == EGroundSpeedMode::Walking)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Walking,
			0);
	}
	else if (OldGroundSpeedMode == EGroundSpeedMode::Jogging)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Jogging,
			0);
	}
	else if (OldGroundSpeedMode == EGroundSpeedMode::Running)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Running,
			0);
	}

	if (NewGroundSpeedMode == EGroundSpeedMode::Walking)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Walking,
			1);
	}
	else if (NewGroundSpeedMode == EGroundSpeedMode::Jogging)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Jogging,
			1);
	}
	else if (NewGroundSpeedMode == EGroundSpeedMode::Running)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(EscapeChroniclesGameplayTags::Status_Movement_Mode_Running,
			1);
	}
}

void AEscapeChroniclesCharacter::OnHealthChanged(const FOnAttributeChangeData& OnHealthChangeData)
{
	UpdateFaintedState();
}

void AEscapeChroniclesCharacter::UpdateFaintedState()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	const UVitalAttributeSet* VitalAttributeSet = AbilitySystemComponent->GetSet<UVitalAttributeSet>();

	if (!IsValid(VitalAttributeSet))
	{
		return;
	}

	const bool bFainted = VitalAttributeSet->GetHealth() <= 0;

	MeshComponent->SetSimulatePhysics(bFainted);
	MeshComponent->bBlendPhysics = bFainted;

	if (bFainted)
	{
		MeshComponent->WakeAllRigidBodies();

		if (!ensureAlways(!FaintedGameplayEffectClass.IsNull()))
		{
			return;
		}

		if (!LoadFaintedGameplayEffectClassHandle.IsValid())
		{
			LoadFaintedGameplayEffectClassHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				FaintedGameplayEffectClass.ToSoftObjectPath(),
				FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnFaintedGameplayEffectClassLoaded));
		}
		else if (FaintedGameplayEffectClass.IsValid())
		{
			OnFaintedGameplayEffectClassLoaded(LoadFaintedGameplayEffectClassHandle);
		}
	}
	else
	{
		MeshComponent->PutAllRigidBodiesToSleep();

		if (FaintedGameplayEffectHandle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(FaintedGameplayEffectHandle);
			FaintedGameplayEffectHandle.Invalidate();
		}
	}
}

void AEscapeChroniclesCharacter::OnFaintedGameplayEffectClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle)
{
#if DO_CHECK
	check(FaintedGameplayEffectClass.IsValid());
	check(LoadObjectHandle.IsValid());
#endif

	// Avoid re-applying the gameplay effect
	if (!FaintedGameplayEffectHandle.IsValid())
	{
		UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

		if (IsValid(AbilitySystemComponent))
		{
			FaintedGameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(
				FaintedGameplayEffectClass->GetDefaultObject<UGameplayEffect>(), 1, FGameplayEffectContextHandle());
		}
	}

	LoadObjectHandle->ReleaseHandle();
	LoadObjectHandle.Reset();
}

bool AEscapeChroniclesCharacter::HasAnyMeshControllingStateTags() const
{
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return false;
	}

	return AbilitySystemComponent->HasAnyMatchingGameplayTags(MeshControllingStateTags);
}

void AEscapeChroniclesCharacter::UpdateMeshControllingState(const FGameplayTag GameplayTag, int32 Count)
{
	const FName MovementModeName = CharacterMoverComponent->GetMovementModeName();

	if (HasAnyMeshControllingStateTags())
	{
		CharacterMoverComponent->DisableMovement();

		MeshComponent->SetCollisionProfileName(FName("Ragdoll"));
		CapsuleComponent->SetCollisionProfileName(FName("NoCollision"));
	}
	else if (MovementModeName == UEscapeChroniclesCharacterMoverComponent::NullModeName)
	{
		CharacterMoverComponent->SetDefaultMovementMode();

		CapsuleComponent->SetCollisionProfileName(DefaultCapsuleCollisionProfileName);
		MeshComponent->SetCollisionProfileName(DefaultMeshCollisionProfileName);

		MoveCapsuleToMesh();

		/** TODO: 
		 * MeshComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
		 * MeshComponent->SetRelativeTransform(InitialMeshTransform);
		 */
	}
}

void AEscapeChroniclesCharacter::MoveCapsuleToMesh()
{
	FVector NewCapsuleLocation = MeshComponent->GetComponentLocation();

	const FVector TraceStart = NewCapsuleLocation;
	FVector TraceEnd = NewCapsuleLocation;
	TraceEnd.Z -= CapsuleComponent->GetScaledCapsuleHalfHeight() / 2;

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	FHitResult OutHit;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
	{
		NewCapsuleLocation = OutHit.Location;
		NewCapsuleLocation.Z += CapsuleComponent->GetScaledCapsuleHalfHeight();
	}

	SetActorLocation(NewCapsuleLocation);
}
