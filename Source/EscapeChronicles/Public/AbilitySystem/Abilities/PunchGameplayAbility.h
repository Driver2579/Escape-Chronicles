// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "PunchGameplayAbility.generated.h"

struct FStreamableHandle;

USTRUCT(BlueprintType)
struct FPunchConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	FName DamagingComponentTag;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> AnimMontage;
};

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
	FGameplayTag StartDamageFrameEventTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag EndDamageFrameEventTag;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<FPunchConfiguration> MontagesQueue;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> SuccessfulPunchEffectClass;

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UGameplayEffect> UnsuccessfulPunchEffectClass;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag BlockingPunchesTag; 
	
	int32 LastPlayedMontageIndex = 0;

	TWeakObjectPtr<UShapeComponent> MeleeHitComponent;

	void LoadAndPlayAnimMontage(int32 Index);
	void LoadGameplayEffects();

	void OnAnimMontageLoaded(const int32 Index);
	
	void OnStartDamageFrame(FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData);
	void OnEndDamageFrame(FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData);

	UFUNCTION()
	void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	FDelegateHandle OnStartDamageFrameEventTagDelegateHandle;
	FDelegateHandle OnEndDamageFrameEventTagDelegateHandle;

	TSharedPtr<FStreamableHandle> LoadCurrentAnimMontageHandle;
	TSharedPtr<FStreamableHandle> LoadSuccessfulPunchEffectHandle;
	TSharedPtr<FStreamableHandle> LoadUnsuccessfulPunchEffectHandle;

	TSoftClassPtr<UGameplayEffect> DesiredToApplyGameplayEffectClass;
	TObjectPtr<UAbilitySystemComponent> InstigatorAbilitySystemComponent;
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	void ApplyDesiredGameplayEffect();
	
	bool WasHitBeforeEffectLoaded = false;
};
