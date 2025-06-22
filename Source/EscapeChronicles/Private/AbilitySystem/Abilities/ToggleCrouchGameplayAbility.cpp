// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/ToggleCrouchGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"

UToggleCrouchGameplayAbility::UToggleCrouchGameplayAbility()
{
	/**
	 * Crouching is replicated through the CharacterMoverComponent. We don't need to replicate it through the ability
	 * system.
	 */
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UToggleCrouchGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (!ensureAlways(IsValid(Character)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	if (!Character->GetCharacterMoverComponent()->IsCrouching())
	{
		Character->Crouch();
	}
	else
	{
		Character->UnCrouch();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}