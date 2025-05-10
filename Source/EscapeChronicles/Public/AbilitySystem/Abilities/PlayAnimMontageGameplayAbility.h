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

private:
	UPROPERTY(EditDefaultsOnly)
	EPlayingMethod PlayingMethod = EPlayingMethod::Queue;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UAnimMontage>> AvailableMontages;

	int32 LastPlayedMontage = 0;
	
	void PlayRandomMontage(UAnimInstance* AnimInstance);
	void PlayQueueMontage(UAnimInstance* AnimInstance);
};