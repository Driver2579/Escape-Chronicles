// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/SetFocusStateTreeTask.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"

FSetFocusStateTreeTask::FSetFocusStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FSetFocusStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.AIController);
	check(InstanceData.FocusActor);
#endif

	// Set the focus on the AI controller to the specified actor
	InstanceData.AIController->SetFocus(InstanceData.FocusActor);

	return EStateTreeRunStatus::Succeeded;
}

void FSetFocusStateTreeTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Clear the focus if we should
	if (InstanceData.bClearFocusOnTaskExit)
	{
		InstanceData.AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}
}