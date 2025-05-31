// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetSuspiciousCharactersStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Common/Structs/FunctionLibriries/SharedRelationshipAttributeSetFunctionLibrary.h"

FGetSuspiciousCharactersStateTreeTask::FGetSuspiciousCharactersStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetSuspiciousCharactersStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	TArray<AEscapeChroniclesCharacter*> SuspiciousCharacters;
	FSharedRelationshipAttributeSetFunctionLibrary::FindSuspiciousCharactersInActors(InstanceData.ActorsToSearchIn,
		SuspiciousCharacters, InstanceData.MinSuspicious);

	// If no suspicious characters were found, return Failed
	if (SuspiciousCharacters.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	/**
	 * We used an array with a raw pointer type because it doesn't compile otherwise. So to use an array with
	 * TObjectPtr, we use MoveTemp. This will cast an array and avoid the copies.
	 */
	InstanceData.OutSuspiciousCharacters = MoveTemp(SuspiciousCharacters);

	return EStateTreeRunStatus::Succeeded;
}