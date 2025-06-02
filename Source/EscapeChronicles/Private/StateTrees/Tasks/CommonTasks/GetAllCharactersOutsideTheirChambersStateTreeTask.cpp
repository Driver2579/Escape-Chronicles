// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetAllCharactersOutsideTheirChambersStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Actors/Triggers/PrisonerChamberZone.h"

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

	/**
	 * We used an array with a raw pointer type because it doesn't compile otherwise. So to use an array with
	 * TObjectPtr, we use MoveTemp. This will cast an array and avoid the copies.
	 */
	InstanceData.OutCharactersOutsideTheirChambers = MoveTemp(CharactersOutsideTheirChambers);

	return EStateTreeRunStatus::Succeeded;
}