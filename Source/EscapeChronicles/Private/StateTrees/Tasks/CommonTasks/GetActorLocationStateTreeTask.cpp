// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetActorLocationStateTreeTask.h"

#include "StateTreeExecutionContext.h"

FGetActorLocationStateTreeTask::FGetActorLocationStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetActorLocationStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Actor);
#endif

	InstanceData.OutLocation = InstanceData.Actor->GetActorLocation();

	return EStateTreeRunStatus::Succeeded;
}