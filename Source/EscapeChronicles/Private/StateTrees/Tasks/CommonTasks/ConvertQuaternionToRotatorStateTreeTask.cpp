// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/ConvertQuaternionToRotatorStateTreeTask.h"

#include "StateTreeExecutionContext.h"

FConvertQuaternionToRotatorStateTreeTask::FConvertQuaternionToRotatorStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FConvertQuaternionToRotatorStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.OutRotation = InstanceData.Quaternion.Rotator();

	return EStateTreeRunStatus::Succeeded;
}