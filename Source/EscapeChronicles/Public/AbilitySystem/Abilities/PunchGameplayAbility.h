// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "PunchGameplayAbility.generated.h"

struct FStreamableHandle;

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
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag WaitGameplayEventTag;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftObjectPtr<UAnimMontage>> MontagesQueue;

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> SuccessfulDamagingEffectClass;

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> UnsuccessfulDamagingEffectClass;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag BlockingPunchesTag; 
	
	int32 LastPlayedMontageIndex = 0;

	void LoadAndPlayAnimMontage(int32 Index);
	void LoadDamagingGameplayEffects();
	
	void OnAnimMontageLoaded(const int32 Index);

	void OnWaitGameplayEventSent(FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData);

	// Handles
	TSharedPtr<FStreamableHandle> LoadCurrentAnimMontageHandle;
	TSharedPtr<FStreamableHandle> LoadSuccessfulDamagingEffectHandle;
	TSharedPtr<FStreamableHandle> LoadUnsuccessfulDamagingEffectHandle;
	FDelegateHandle WaitGameplayEventDelegateHandle;

	bool WasEventCalledBeforeEffectLoaded = false;

	TSoftClassPtr<UGameplayEffect> DesiredToApplyGameplayEffectClass;
	TObjectPtr<UAbilitySystemComponent> InstigatorAbilitySystemComponent;
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;
	
	void ApplyDesiredGameplayEffect();
};
