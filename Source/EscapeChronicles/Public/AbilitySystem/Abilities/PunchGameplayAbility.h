// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "PunchGameplayAbility.generated.h"

struct FStreamableHandle;

// Defines the configuration for a single punch animation and its associated collider
USTRUCT(BlueprintType)
struct FPunchConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FName DamagingColliderTag;

	TWeakObjectPtr<UShapeComponent> DamagingCollider;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> AnimMontage;
};

// Ability class for executing a punch with animation, damage effects, and audio feedback
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
	// Gameplay event tag used to signal the start of the damage window during the animation
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag StartDamageFrameEventTag;

	// Gameplay event tag used to signal the end of the damage window during the animation
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag EndDamageFrameEventTag;

	// A queue of punch configurations with montages and colliders
	UPROPERTY(EditDefaultsOnly)
	TArray<FPunchConfiguration> MontagesQueue;

	// Gameplay effect to apply on a successful punch (dealing damage)
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> SuccessfulPunchEffectClass;

	// Gameplay effect to apply on an unsuccessful punch (blocked)
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> UnsuccessfulPunchEffectClass;

	// Sound to play on a successful punch
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USoundCue> SuccessfulPunchSound;

	// Sound to play on a failed punch
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USoundCue> UnsuccessfulPunchSound;

	// Tag that marks actors or states that block punches
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag BlockingPunchesTag; 

	// Initializes the collider used to detect punch hits
	void SetupDamagingCollider();

	// Begins listening for gameplay events
	void BeginWaitGameplayEvents();

	// Async load assets
	void LoadAndPlayAnimMontage();
	void LoadGameplayEffects();
	void LoadSounds();

	// Handling asset events
	void OnAnimMontageLoaded();
	void OnAnimMontageBlendingOut(UAnimMontage* AnimMontage, bool bInterrupted);
	void OnGameplayEffectLoaded(const TSoftClassPtr<UGameplayEffect> LoadedEffect);
	void OnSoundLoaded(const TSoftObjectPtr<USoundCue> LoadedSound);

	// Called when the punch hitbox overlaps another actor's collider
	UFUNCTION()
	void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Applies the desired gameplay effect depending on whether the punch was successful
	void ApplyDesiredGameplayEffect() const;

	// Unloads a previously loaded asset using its handle
	static void UnloadByHandle(TSharedPtr<FStreamableHandle>& Handle);

	// Delegate handles for gameplay event tags (used to remove safely)
	FDelegateHandle OnStartDamageFrameEventTagDelegateHandle;
	FDelegateHandle OnEndDamageFrameEventTagDelegateHandle;

	// Handles for managing asset streaming operations (used to remove safely)
	TSharedPtr<FStreamableHandle> LoadCurrentAnimMontageHandle;
	TSharedPtr<FStreamableHandle> LoadSuccessfulPunchEffectHandle;
	TSharedPtr<FStreamableHandle> LoadUnsuccessfulPunchEffectHandle;
	TSharedPtr<FStreamableHandle> LoadSuccessfulPunchSoundHandle;
	TSharedPtr<FStreamableHandle> LoadUnsuccessfulPunchSoundHandle;

	// Ability system component references for instigator and target
	//TObjectPtr<UAbilitySystemComponent> InstigatorAbilitySystemComponent;
	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	// Cache to be applied after punch resolution
	TSoftClassPtr<UGameplayEffect> DesiredToApplyGameplayEffectClass;
	TSoftObjectPtr<USoundCue> DesiredToPlaySound;
	TWeakObjectPtr<UShapeComponent> DesiredDamagingCollider;

	// Index of the current punch configuration being executed
	int32 CurrentConfigurationIndex = 0;

	// Flag indicating whether the punch connected successfully
	bool IsPunchHappened = false;
};
