// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/FindSmartObjectByClassStateTreeTask.h"

#include "SmartObjectRequestTypes.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "Kismet/GameplayStatics.h"

FFindSmartObjectByClassStateTreeTask::FFindSmartObjectByClassStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FFindSmartObjectByClassStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!ensureAlways(IsValid(InstanceData.ActorClass)))
	{
		return EStateTreeRunStatus::Failed;
	}

	const UWorld* World = Context.GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Get all actors of the specified class
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, InstanceData.ActorClass, FoundActors);

	const USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();

	// Ensure the SmartObjectSubsystem is valid
	if (!ensureAlways(IsValid(SmartObjectSubsystem)))
	{
		return EStateTreeRunStatus::Failed;
	}

	// Find smart objects in the list of found actors
	TArray<FSmartObjectRequestResult> FindSmartObjectsResults;
	SmartObjectSubsystem->FindSmartObjectsInList(InstanceData.SmartObjectRequestFilter, FoundActors,
		FindSmartObjectsResults,
		FConstStructView::Make(FSmartObjectActorUserData(InstanceData.UserActor)));

	// If no smart objects were found, return failed status
	if (FindSmartObjectsResults.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	// Return the first found smart object
	InstanceData.OutSmartObjectRequestResult = FindSmartObjectsResults[0];

	return EStateTreeRunStatus::Succeeded;
}