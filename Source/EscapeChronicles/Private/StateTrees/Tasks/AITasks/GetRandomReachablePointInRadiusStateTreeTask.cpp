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

	const FVector PawnLocation = InstanceData.Pawn->GetActorLocation();

	FNavLocation FoundLocation;

	// We pass nullptr into the NavData, so it will use the MainNavData we used above
	bool bResult = NavigationSystem->GetRandomReachablePointInRadius(PawnLocation, InstanceData.Radius,
		FoundLocation, nullptr,	QueryFilter);

	/**
	 * If we failed to find a random reachable point in the specified radius, then try to find a random point in the
	 * navigable radius without checking if it's reachable instead, if we are allowed to do so.
	 */
	if (!bResult && InstanceData.bGetRandomPointInNavigableRadiusIfFailed)
	{
		bResult = NavigationSystem->GetRandomPointInNavigableRadius(PawnLocation, InstanceData.Radius, FoundLocation,
			nullptr, QueryFilter);
	}

	/**
	 * If we still didn't find the point, then the bot is probably outside of the navmesh or we were not allowed to find
	 * the random point in navigable space without checking for reachability. In this case, try to use the
	 * ProjectPointToNavigation function to get him a point that matches the query filter even if it's outside of the
	 * given radius, but only if we are allowed to do so.
	 */
	if (!bResult && InstanceData.bProjectPointOnNavigationIfFailed)
	{
		bResult = NavigationSystem->ProjectPointToNavigation(PawnLocation, FoundLocation,
			InstanceData.ProjectPointToNavigationExtent, nullptr, QueryFilter);
	}

	// Check if we found a random reachable point in the specified radius
	if (!bResult)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Return the found location
	InstanceData.OutRandomPoint = FoundLocation;

	return EStateTreeRunStatus::Succeeded;
}