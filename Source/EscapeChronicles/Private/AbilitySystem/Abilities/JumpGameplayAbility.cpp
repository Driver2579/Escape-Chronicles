// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/JumpGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"

UJumpGameplayAbility::UJumpGameplayAbility()
{
	// Jump is replicated through the CharacterMoverComponent. We don't need to replicate it through the ability system.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UJumpGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor.Get(),
		ECastCheckedType::NullAllowed);

	if (ensureAlways(IsValid(Character)))
	{
		Character->Jump();
	}
}

void UJumpGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor.Get(),
		ECastCheckedType::NullAllowed);

	if (IsValid(Character))
	{
		Character->StopJumping();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}