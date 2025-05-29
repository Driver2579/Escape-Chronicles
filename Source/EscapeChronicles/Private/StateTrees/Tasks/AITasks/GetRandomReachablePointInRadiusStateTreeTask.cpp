// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/GetRandomReachablePointInRadiusStateTreeTask.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "NavFilters/NavigationQueryFilter.h"

FGetRandomReachablePointInRadiusStateTreeTask::FGetRandomReachablePointInRadiusStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetRandomReachablePointInRadiusStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(Context.GetWorld());

	if (!ensureAlways(IsValid(NavigationSystem)))
	{
		return EStateTreeRunStatus::Failed;
	}

	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	FSharedConstNavQueryFilter QueryFilter;

	// Get the query filter if the class for it is specified and the NavData is valid. It will be left empty otherwise.
	if (IsValid(InstanceData.QueryFilterClass) && ensureAlways(NavigationSystem->MainNavData))
	{
		QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavigationSystem->MainNavData,
			InstanceData.AIController, InstanceData.QueryFilterClass);
	}

	// We pass nullptr into the NavData, so it will use the MainNavData we used above
	FNavLocation FoundLocation;
	const bool bResult = NavigationSystem->GetRandomReachablePointInRadius(InstanceData.Pawn->GetActorLocation(),
		InstanceData.Radius, FoundLocation, nullptr, QueryFilter);

	// Return Failed if we didn't find any point
	if (!bResult)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Return the found location
	InstanceData.OutRandomPoint = FoundLocation;

	return EStateTreeRunStatus::Succeeded;
}