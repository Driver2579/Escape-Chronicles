// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/ArrayContainsEmptyPrisonerChambersStateTreeCondition.h"

#include "StateTreeExecutionContext.h"
#include "Actors/Triggers/PrisonerChamberZone.h"

bool FArrayContainsEmptyPrisonerChambersStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	
	for (const AActor* Actor : InstanceData.ActorsToCheck)
	{
		const APrisonerChamberZone* PrisonerChamberZone = Cast<APrisonerChamberZone>(Actor);

		// Check if an actor is a valid PrisonerChamberZone
		if (!IsValid(PrisonerChamberZone))
		{
			continue;
		}

		bool bHasOwningCharacter;

		// Check if this chamber has an owning character that is currently in the game but isn't in this chamber
		if (!PrisonerChamberZone->IsOwningCharacterInThisChamber(bHasOwningCharacter) && bHasOwningCharacter)
		{
			// If so, then return true. If bInvert is true, then false will be returned.
			return !bInvert;
		}
	}

	// Return false if we didn't find any empty chambers. If bInvert is true, then true will be returned.
	return bInvert;
}