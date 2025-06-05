// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueInterface.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "PunchGameplayAbility.generated.h"

struct FStreamableHandle;

// Defines the configuration for a single punch animation and its associated collision
USTRUCT(BlueprintType)
struct FPunchConfiguration
{
	GENERATED_BODY()

	// Playable animation for the realization of this punch
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> AnimMontage;

	// A component with this tag will be written to DamageCollision (its overlaps generate punch)
	UPROPERTY(EditDefaultsOnly)
	FName DamageCollisionTag;

	// Overlap collision, which is used to determine when the punch happens
	TWeakObjectPtr<UPrimitiveComponent> DamageCollision;
};

// Ability class for executing a punch with animation, damage effects, and gameplay cue feedback
UCLASS()
class ESCAPECHRONICLES_API UPunchGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	// Gameplay tag that is used to signal the start of the damage window during the animation
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag StartDamageFrameEventTag;

	// Gameplay tag that is used to signal the end of the damage window during the animation
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag EndDamageFrameEventTag;

	// A queue of punch configurations with montages and collisions. Each new punch will have an index of 1 more.
	UPROPERTY(EditDefaultsOnly)
	TArray<FPunchConfiguration> MontagesQueue;

	// Gameplay effect to apply on a successful punch (dealing damage)
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> SuccessfulPunchGameplayEffectClass;

	// Gameplay effect to apply on an unsuccessful punch (blocked)
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> UnsuccessfulPunchGameplayEffectClass;

	// Gameplay cue to apply on a successful punch (dealing damage)
	UPROPERTY(EditDefaultsOnly)
	FGameplayCueTag SuccessfulPunchGameplayCueTag;

	// Gameplay cue to apply on an unsuccessful punch (blocked)
	UPROPERTY(EditDefaultsOnly)
	FGameplayCueTag UnsuccessfulPunchGameplayCueTag;

	// Actors with these tags will block punches
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer BlockingPunchesTags; 

	// Initializes the collision used to detect punch hits
	bool SetupDamageCollision();

	// Begins listening for gameplay events
	void RegisterPunchGameplayEvents();

	// Async load assets
	bool LoadAndPlayAnimMontage();
	bool LoadGameplayEffects();

	// Handles asset events
	void OnAnimMontageLoaded();
	void OnAnimMontageBlendingOut(UAnimMontage* AnimMontage, bool bInterrupted);
	void OnGameplayEffectLoaded(const TSoftClassPtr<UGameplayEffect> LoadedEffect);

	// Called when the punch hitbox overlaps another actor's collision
	UFUNCTION()
	void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Applies the desired gameplay effect depending on whether the punch was successful
	void ApplyDesiredGameplayEffectToTargetChecked() const;

	// Unloads a previously loaded asset using its handle
	static void UnloadSoftObject(TSharedPtr<FStreamableHandle>& Handle);

	// Delegate handles for gameplay event tags (used to remove safely)
	FDelegateHandle OnStartDamageFrameEventTagDelegateHandle;
	FDelegateHandle OnEndDamageFrameEventTagDelegateHandle;

	// Handles for managing asset streaming operations (used to remove safely)
	TSharedPtr<FStreamableHandle> LoadCurrentAnimMontageHandle;
	TSharedPtr<FStreamableHandle> LoadSuccessfulPunchEffectHandle;
	TSharedPtr<FStreamableHandle> LoadUnsuccessfulPunchEffectHandle;

	// Ability system component reference for the target
	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	// Cached collision component used to detect overlap with the punch target
	TWeakObjectPtr<UPrimitiveComponent> DesiredDamageCollision;

	// Cached gameplay effect to apply to the target after punch hit 
	TSoftClassPtr<UGameplayEffect> DesiredGameplayEffectClassToApply;

	// Index of the current punch configuration being executed
	int32 CurrentConfigurationIndex = 0;

	// A flag indicating whether the punch was successful
	bool bPunchHappened = false;
};
