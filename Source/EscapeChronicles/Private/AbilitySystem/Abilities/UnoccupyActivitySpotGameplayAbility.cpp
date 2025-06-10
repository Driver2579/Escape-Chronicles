// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/UnoccupyActivitySpotGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

UUnoccupyActivitySpotGameplayAbility::UUnoccupyActivitySpotGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UUnoccupyActivitySpotGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

#if DO_CHECK
	check(ActorInfo);
	check(ActorInfo->OwnerActor.IsValid());
#endif

	const AEscapeChroniclesPlayerState* PlayerState = Cast<AEscapeChroniclesPlayerState>(ActorInfo->OwnerActor);

	if (!ensureAlways(IsValid(PlayerState)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);

		return;
	}

	AActivitySpot* OccupyingActivitySpot = PlayerState->GetOccupyingActivitySpot();

	if (ensureAlways(IsValid(OccupyingActivitySpot)) && OccupyingActivitySpot->SetOccupyingCharacter(nullptr))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);

		return;
	}

	CancelAbility(Handle, ActorInfo, ActivationInfo, false);
}
