// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/ArrayContainsSuspiciousCharactersStateTreeCondition.h"

#include "StateTreeExecutionContext.h"
#include "Common/Structs/FunctionLibriries/SharedRelationshipAttributeSetFunctionLibrary.h"

bool FArrayContainsSuspiciousCharactersStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Find the first suspicious character in the given array of actors
	const AEscapeChroniclesCharacter* SuspiciousCharacter =
		FSharedRelationshipAttributeSetFunctionLibrary::FindSuspiciousCharacterInActor(InstanceData.ActorsToCheck,
			InstanceData.MinSuspicious);

	// If we found a suspicious character, return true or false based on the bInvert flag
	return (SuspiciousCharacter != nullptr) ^ bInvert;
}