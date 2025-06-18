// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/ArrayContainsPrisonersOutsideTheirChambersStateTreeCondition.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Actors/Triggers/PrisonerChamberZone.h"
#include "Characters/EscapeChroniclesCharacter.h"

bool FArrayContainsPrisonersOutsideTheirChambersStateTreeCondition::TestCondition(
	FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	for (AActor* Actor : InstanceData.ActorsToCheck)
	{
		const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(Actor);

		// Check if an actor is a valid character
		if (!IsValid(Character))
		{
			continue;
		}

		const UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

		// Check if the character is allowed to be outside his chamber by checking if he has any of the allowed tags
		const bool bCharacterHasAllowedTag = IsValid(AbilitySystemComponent) &&
			AbilitySystemComponent->HasAnyMatchingGameplayTags(InstanceData.AllowedGameplayTags);

		// Skip the character if he has any of the allowed tags
		if (bCharacterHasAllowedTag)
		{
			continue;
		}

		bool bHasOwningChamber;

		// Check if the character is in his own chamber and if he has any owning chamber
		if (!APrisonerChamberZone::IsCharacterInHisChamber(Character, bHasOwningChamber) && bHasOwningChamber)
		{
			// Return true if yes. If bInvert is true, then false will be returned instead.
			return !bInvert;
		}
	}

	/**
	 * Return false if we didn't find any character outside their chambers. If bInvert is true, then true will be
	 * returned instead.
	 */
	return bInvert;
}