// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetAllCharactersOutsideTheirChambersStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Actors/Triggers/PrisonerChamberZone.h"
#include "Characters/EscapeChroniclesCharacter.h"

class UAbilitySystemComponent;

FGetAllCharactersOutsideTheirChambersStateTreeTask::FGetAllCharactersOutsideTheirChambersStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetAllCharactersOutsideTheirChambersStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	const UWorld* World = Context.GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return EStateTreeRunStatus::Failed;
	}

	TArray<AEscapeChroniclesCharacter*> CharactersOutsideTheirChambers;
	APrisonerChamberZone::GetAllCharactersOutsideTheirChambers(World, CharactersOutsideTheirChambers);

	// If no characters were found, return Failed
	if (CharactersOutsideTheirChambers.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.AllowedGameplayTags.IsValid())
	{
		for (int32 i = CharactersOutsideTheirChambers.Num() - 1; i >= 0; --i)
		{
			const UAbilitySystemComponent* AbilitySystemComponent =
				CharactersOutsideTheirChambers[i]->GetAbilitySystemComponent();

			// Check if the character is allowed to be outside his chamber by checking if he has any of the allowed tags
			const bool bCharacterHasAllowedTag = IsValid(AbilitySystemComponent) &&
				AbilitySystemComponent->HasAnyMatchingGameplayTags(InstanceData.AllowedGameplayTags);

			// Remove the character from the output array if he has any of the allowed tags
			if (bCharacterHasAllowedTag)
			{
				CharactersOutsideTheirChambers.RemoveAt(i, EAllowShrinking::No);
			}
		}

		// If no characters have left, return Failed
		if (CharactersOutsideTheirChambers.IsEmpty())
		{
			return EStateTreeRunStatus::Failed;
		}
	}

	/**
	 * We used an array with a raw pointer type because it doesn't compile otherwise. So to use an array with
	 * TObjectPtr, we use MoveTemp. This will cast an array and avoid the copies.
	 */
	InstanceData.OutCharactersOutsideTheirChambers = MoveTemp(CharactersOutsideTheirChambers);

	return EStateTreeRunStatus::Succeeded;
}