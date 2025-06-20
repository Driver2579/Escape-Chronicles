// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/FindSmartObjectFromActorStateTreeTask.h"

#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"

FFindSmartObjectFromActorStateTreeTask::FFindSmartObjectFromActorStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FFindSmartObjectFromActorStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Actor);
#endif

	const USmartObjectSubsystem* SmartObjectSubsystem = Context.GetWorld()->GetSubsystem<USmartObjectSubsystem>();

	if (!ensureAlways(IsValid(SmartObjectSubsystem)))
	{
		return EStateTreeRunStatus::Failed;
	}

	/**
	 * Find the smart objects in one specified actor. There is no function version that accepts a single actor, so we
	 * use the version that accepts an array but with only one actor in it.
	 */
	TArray<FSmartObjectRequestResult> FindSmartObjectsResult;
	SmartObjectSubsystem->FindSmartObjectsInList(InstanceData.SmartObjectRequestFilter,
		TArray({ InstanceData.Actor }), FindSmartObjectsResult,
		FConstStructView::Make(FSmartObjectActorUserData(InstanceData.UserActor)));

	// If no smart objects were found, return failed status
	if (FindSmartObjectsResult.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	// Set the first found smart object as the result
	InstanceData.OutSmartObjectRequestResult = FindSmartObjectsResult[0];

	// Since we found a smart object, we can return the Succeeded status
	return EStateTreeRunStatus::Succeeded;
}