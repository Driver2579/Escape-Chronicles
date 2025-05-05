// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/OverrideGroundSpeedModeGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Enums/Mover/GroundSpeedMode.h"

UOverrideGroundSpeedModeGameplayAbility::UOverrideGroundSpeedModeGameplayAbility()
	: GroundSpeedMode(EGroundSpeedMode::None)
{
	/**
	 * Ground speed mode is replicated through the character and CharacterMoverComponent. We don't need to replicate it
	 * through the ability system.
	 */
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UOverrideGroundSpeedModeGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		Character->OverrideGroundSpeedMode(GroundSpeedMode);
	}
}

void UOverrideGroundSpeedModeGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UOverrideGroundSpeedModeGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor.Get(),
		ECastCheckedType::NullAllowed);

	if (IsValid(Character))
	{
		Character->ResetGroundSpeedMode(GroundSpeedMode);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}