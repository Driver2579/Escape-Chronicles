// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/CastActorsArrayToCharactersArrayStateTreeTask.h"

#include "StateTreeExecutionContext.h"

FCastActorsArrayToCharactersArrayStateTreeTask::FCastActorsArrayToCharactersArrayStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FCastActorsArrayToCharactersArrayStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	InstanceData.Characters = reinterpret_cast<TArray<TObjectPtr<AEscapeChroniclesCharacter>>&>(InstanceData.Actors);

	return EStateTreeRunStatus::Succeeded;
}