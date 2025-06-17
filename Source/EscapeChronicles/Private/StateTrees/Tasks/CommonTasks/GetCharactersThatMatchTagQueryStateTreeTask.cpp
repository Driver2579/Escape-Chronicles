// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetCharactersThatMatchTagQueryStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"

FGetCharactersThatMatchTagQueryStateTreeTask::FGetCharactersThatMatchTagQueryStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetCharactersThatMatchTagQueryStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Find all characters in the provided actors array that match the tag query
	for (AActor* Actor : InstanceData.ActorsToSearchIn)
	{
		AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(Actor);

		if (!IsValid(Character))
		{
			continue;
		}

		const UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

		// Check if the character has a valid ASC and it matches the tag query
		const bool bCharacterMatchesTagQuery = IsValid(AbilitySystemComponent) &&
			AbilitySystemComponent->MatchesGameplayTagQuery(InstanceData.GameplayTagQuery);

		// Add the character to the output array if it matches the tag query
		if (bCharacterMatchesTagQuery)
		{
			InstanceData.OutSuspiciousCharacters.Add(Character);
		}
	}

	// Return Succeeded or Failed depending on whether any characters that match the tag query were found
	return !InstanceData.OutSuspiciousCharacters.IsEmpty() ?
		EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}