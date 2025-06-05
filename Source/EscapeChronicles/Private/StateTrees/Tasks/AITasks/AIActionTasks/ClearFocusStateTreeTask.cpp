// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/ClearFocusStateTreeTask.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"

FClearFocusStateTreeTask::FClearFocusStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FClearFocusStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.AIController);
#endif

	InstanceData.AIController->ClearFocus(EAIFocusPriority::Gameplay);

	return EStateTreeRunStatus::Succeeded;
}