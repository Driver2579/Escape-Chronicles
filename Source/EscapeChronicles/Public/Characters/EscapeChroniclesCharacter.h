// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MoverSimulationTypes.h"
#include "AbilitySystemInterface.h"
#include "Interfaces/Saveable.h"
#include "ActiveGameplayEffectHandle.h"
#include "Common/Enums/Mover/GroundSpeedMode.h"
#include "EscapeChroniclesCharacter.generated.h"

class UInventoryManagerComponent;
class UBoxComponent;
class UInteractionManagerComponent;
class UEscapeChroniclesCharacterMoverComponent;
class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;
class UNavMoverComponent;

struct FGameplayEffectSpec;
struct FOnAttributeChangeData;

enum class EStanceMode : uint8;
enum class EGroundSpeedMode : uint8;

UCLASS(Config=Game)
class AEscapeChroniclesCharacter : public APawn, public IMoverInputProducerInterface, public IAbilitySystemInterface,
	public ISaveable
{
	GENERATED_BODY()

public:
	AEscapeChroniclesCharacter();

	// Returns CapsuleComponent subobject
	UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

	// Returns MeshComponent subobject
	USkeletalMeshComponent* GetMesh() const { return MeshComponent; }

#if WITH_EDITORONLY_DATA
	// Returns ArrowComponent subobject
	class UArrowComponent* GetArrowComponent() const { return ArrowComponent; }
#endif

	// Returns CameraBoomComponent subobject
	USpringArmComponent* GetCameraBoomComponent() const { return CameraBoomComponent; }

	// Returns FollowCameraComponent subobject
	UCameraComponent* GetFollowCameraComponent() const { return FollowCameraComponent; }

	// Returns InteractionManagerComponent subobject
	UInteractionManagerComponent* GetInteractionManagerComponent() const { return InteractionManagerComponent; }

	// Returns CharacterMoverComponent subobject
	UEscapeChroniclesCharacterMoverComponent* GetCharacterMoverComponent() const { return CharacterMoverComponent; }

	// Returns NavMoverComponent subobject
	UNavMoverComponent* GetNavMoverComponent() const { return NavMoverComponent; }

	virtual UInventoryManagerComponent* GetInventoryManagerComponent() const { return InventoryManagerComponent; }

	virtual void Tick(float DeltaSeconds) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override final;
	class UEscapeChroniclesAbilitySystemComponent* GetEscapeChroniclesAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable)
	bool IsTurning() const { return bTurning; }

	UFUNCTION(BlueprintCallable)
	FRotator GetActorAndViewDelta() const { return ActorAndViewDelta; }

	virtual void PostLoad() override;

	virtual FVector GetNavAgentLocation() const override;

	virtual void UpdateNavigationRelevance() override;

	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;
	virtual FVector ConsumeMovementInputVector() override;

	// Should be called for looking input trigger
	void Look(const FVector2D& LookAxisVector);

	// Should be called for movement input trigger
	void Move(FVector MovementVector);

	// Should be called for movement input completion
	void StopMoving();

	// Should be called for jump input trigger
	void Jump();

	// Should be called for jump input completion
	void StopJumping();

	// Should be called for crouch input trigger
	void Crouch();

	// Should be called for crouch input completion
	void UnCrouch();

	// Should be called for any input trigger that wants to override the ground speed mode (e.g., walk, jog, run)
	void OverrideGroundSpeedMode(const EGroundSpeedMode GroundSpeedModeOverride);

	/**
	 * Should be called for any input completion that wants to reset the ground speed mode after overriding it.
	 * @remark The ground speed mode will be reset only after the completion of such an input that was the last one.
	 */
	void ResetGroundSpeedMode(const EGroundSpeedMode GroundSpeedModeOverrideToReset);

protected:
	virtual void BeginPlay() override;

	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

	// Whether we author our movement inputs relative to whatever base we're standing on, or leave them in world space
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	bool bUseBaseRelativeMovement = true;

	// If true, rotate the Character toward the direction the actor is moving
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	bool bOrientRotationToMovement = true;

	// If true, the actor will remain vertical despite any rotation applied to the actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	bool bShouldRemainVertical = true;

	/**
	 * If true, the actor will continue orienting towards the last intended orientation (from input) even after movement
	 * intent input has ceased.
	 * This makes the character finish orienting after a quick stick flick from the player.
	 * If false, the character will not turn without an input.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bMaintainLastInputOrientation = false;

	// When ActorAndViewDelta is greater than this value, the mesh starts to rotate to reduce it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Rotation")
	float AngleToStartTurning = 90;

	// When ActorAndViewDelta is less than this value, the mesh stops rotating
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Rotation")
	float AngleToStopTurning = 10;

	// ActorAndViewDelta interpolation speed when rotating mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Rotation")
	float TurningInterpSpeed = 7;

	// Tags that block mesh rotation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Rotation")
	FGameplayTagContainer BlockTurningTags;

	/**
	 * This effect is triggered when a character falls unconscious. It must be infinite and give the same tag as
	 * “FaintedGameplayTag”
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fainted")
	TSoftClassPtr<class UGameplayEffect> FaintedGameplayEffectClass;

	// Entry point for input production. Authors an input for the next simulation frame.
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	UFUNCTION()
	void OnMoverPostMovement(const FMoverTimeStep& TimeStep, FMoverSyncState& SyncState,
		FMoverAuxStateContext& AuxState);

	UFUNCTION()
	virtual void OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd);

	UFUNCTION()
	virtual void OnMovementModeChanged(const FName& PreviousMovementModeName, const FName& NewMovementModeName);

	UFUNCTION()
	virtual void OnStanceChanged(const EStanceMode OldStance, const EStanceMode NewStance);

	virtual void OnGroundSpeedModeChanged(const EGroundSpeedMode OldGroundSpeedMode,
		const EGroundSpeedMode NewGroundSpeedMode);

	/**
	 * Calls SyncMovementModesTagsWithAbilitySystem, SyncStanceTagsWithAbilitySystem and
	 * SyncGroundSpeedModeTagsWithAbilitySystem. Could be overriden by child classes to add more similar functions.
	 */
	virtual void SyncCharacterMoverComponentTagsWithAbilitySystem() const;

	// Synchronizes all movement modes' tags from CharacterMoverComponent with an ability system component
	virtual void SyncMovementModesTagsWithAbilitySystem() const;

	// Synchronizes all stances' tags from CharacterMoverComponent with an ability system component
	void SyncStancesTagsWithAbilitySystem() const;

	// Synchronizes all ground speed modes' tags from CharacterMoverComponent with an ability system component
	void SyncGroundSpeedModeTagsWithAbilitySystem() const;

private:
	/**
	 * The CapsuleComponent being used for movement collision (by CharacterMovement). Always treated as being vertically
	 * aligned in simple collision check functions.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	// The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

#if WITH_EDITORONLY_DATA
	// Component shown in the editor only to indicate character facing
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UArrowComponent> ArrowComponent;
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Interaction", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteractionManagerComponent> InteractionManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Interaction", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InteractionZone;

	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoomComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FollowCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Movement", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UEscapeChroniclesCharacterMoverComponent> CharacterMoverComponent;

	// Holds functionality for nav movement data and functions
	UPROPERTY(VisibleAnywhere, Transient, Category="Components|Movement|Nav Movement", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UNavMoverComponent> NavMoverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;

	// Movement input (intent or velocity) the last time we had one that wasn't zero
	FVector LastAffirmativeMoveInput = FVector::ZeroVector;

	FVector CachedMoveInputVelocity = FVector::ZeroVector;

	bool bIsJumpJustPressed = false;
	bool bIsJumpPressed = false;

	bool bWantsToBeCrouched = false;

	// Difference between the rotation angle of the mesh and the rotation angle of the actor
	FRotator ActorAndViewDelta;

	// Whether the mesh is turning now
	bool bTurning = false;

	// Mesh rotation on BeginPlay
	FRotator InitialMeshRotation;

	/**
	 * Synchronizes all stances' tags from CharacterMoverComponent with an ability system component based on the passed
	 * values that should be gotten when OnStanceChanged is called.
	 */
	void SyncStancesTagsWithAbilitySystem(const EStanceMode OldStance, const EStanceMode NewStance) const;

	EGroundSpeedMode DesiredGroundSpeedModeOverride;

	/**
	 * Synchronizes all ground speed modes' tags from CharacterMoverComponent with an ability system component based on
	 * the passed values that should be gotten when OnGroundSpeedMode is called.
	 */
	void SyncGroundSpeedModeTagsWithAbilitySystem(const EGroundSpeedMode OldGroundSpeedMode,
		const EGroundSpeedMode NewGroundSpeedMode) const;

	// Makes it a ragdoll if health is 0 or less
	void OnHealthChanged(const FOnAttributeChangeData& OnHealthChangeData);

	/**
	 * Sets the bFainted based on current health (when fainted, the collision of the capsule and the movement are
	 * disabled, and the mesh becomes a ragdoll)
	 */
	void UpdateFaintedState();

	FName DefaultMeshCollisionProfileName;
	FName DefaultCapsuleCollisionProfileName;

	TSharedPtr<FStreamableHandle> LoadFaintedGameplayEffectClassHandle;

	void OnFaintedGameplayEffectClassLoaded();

	FActiveGameplayEffectHandle FaintedGameplayEffectHandle;
};