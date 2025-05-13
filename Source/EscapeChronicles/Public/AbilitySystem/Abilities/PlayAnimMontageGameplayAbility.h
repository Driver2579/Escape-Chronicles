// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "PlayAnimMontageGameplayAbility.generated.h"

UENUM()
enum class EPlayingMethod : uint8
{
	Random,
	Queue
};

UCLASS()
class ESCAPECHRONICLES_API UPlayAnimMontageGameplayAbility : public UEscapeChroniclesGameplayAbility
{
	GENERATED_BODY()

public:
	UPlayAnimMontageGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag GameplayEventTag;

	UPROPERTY(EditDefaultsOnly)
	EPlayingMethod PlayingMethod = EPlayingMethod::Queue;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftObjectPtr<UAnimMontage>> AvailableMontages;

	TSharedPtr<struct FStreamableHandle> LoadCurrentAnimMontageHandle;
	
	int32 LastPlayedMontageIndex = 0;

	void PlayRandomMontage();
	void PlayQueueMontage();

	void LoadAndPlayAnimMontage(int32 Index);
	
	void OnAnimMontageLoaded(const int32 Index);

	void OnDeployBombAnimMontageBlendingOut(UAnimMontage* AnimMontage, bool bInterrupted);

	void OnAnimMontageEnded(UAnimMontage* AnimMontage, bool bInterrupted);

	UFUNCTION()
	void Test(FGameplayEventData Payload);
};