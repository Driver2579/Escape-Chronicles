// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetAIControllerFromPawnStateTreeTask.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"

FGetAIControllerFromPawnStateTreeTask::FGetAIControllerFromPawnStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetAIControllerFromPawnStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

#if DO_CHECK
	check(InstanceData.Pawn);
#endif

	InstanceData.OutAIController = InstanceData.Pawn->GetController<AAIController>();

	return InstanceData.OutAIController ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}
