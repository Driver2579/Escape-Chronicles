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
			bool bHasBedWithSameOwningCharacter;

			/**
			 * If so, then return true if the chamber has no beds with the same owning character that are occupied and
			 * invert the result if needed.
			 */
			return !PrisonerChamberZone->IsOwningCharacterBedOccupied(bHasBedWithSameOwningCharacter) ^ bInvert;
		}
	}

	// Return false if we didn't find any empty chambers. If bInvert is true, then true will be returned.
	return bInvert;
}