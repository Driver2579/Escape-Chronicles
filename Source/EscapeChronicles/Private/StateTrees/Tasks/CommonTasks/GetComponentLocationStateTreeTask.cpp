// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetComponentLocationStateTreeTask.h"

#include "StateTreeExecutionContext.h"

FGetComponentLocationStateTreeTask::FGetComponentLocationStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetComponentLocationStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Component);
#endif

	InstanceData.OutLocation = InstanceData.Component->GetComponentLocation();

	return EStateTreeRunStatus::Succeeded;
}