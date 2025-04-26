// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/CrouchGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"

UCrouchGameplayAbility::UCrouchGameplayAbility()
{
	/**
	 * Crouching is replicated through the CharacterMoverComponent. We don't need to replicate it through the ability
	 * system.
	 */
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UCrouchGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor.Get(),
		ECastCheckedType::NullAllowed);

	if (ensureAlways(IsValid(Character)))
	{
		Character->Crouch();
	}
}

void UCrouchGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UCrouchGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor.Get(),
		ECastCheckedType::NullAllowed);

	if (IsValid(Character))
	{
		Character->UnCrouch();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}