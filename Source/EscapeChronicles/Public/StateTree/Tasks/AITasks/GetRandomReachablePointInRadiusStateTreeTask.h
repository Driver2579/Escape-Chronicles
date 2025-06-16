// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "GetRandomReachablePointInRadiusStateTreeTask.generated.h"

class AAIController;

USTRUCT()
struct FGetRandomReachablePointInRadiusStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;

	// A pawn controlled by the AIController.
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// The radius within the random reachable point will be searched
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0.1))
	float Radius = 1000;

	// Filter to use when searching for points. Can be left empty if you don't need any specific filtering.
	UPROPERTY(EditAnywhere, Category="Parameter")
	TSubclassOf<class UNavigationQueryFilter> QueryFilterClass;

	/**
	 * If true, then in case of failure to find a random reachable point in a specified radius, the random point in the
	 * whole navigable space within the specified radius will be searched (without checking if the point is reachable).
	 * This is needed to prevent the task from failure in case if the bot isn't in the navigable space that matches the
	 * given query filter.
	 */
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bGetRandomPointInNavigableRadiusIfFailed = true;

	/**
	 * If true, then in case of failure to find a random reachable point in a specified radius or a random point in a
	 * navigable radius, the ProjectPointToNavigation function will be called to find a point on the navigation mesh.
	 * This is needed to prevent the task from failure in case if both of the searches have failed. Usually, this
	 * happens if the bot is outside the navmesh.
	 */
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bProjectPointOnNavigationIfFailed = true;

	/**
	 * An extent to use for the ProjectPointToNavigation function in case if the point wasn't found in the specified
	 * radius. Zero vector means the default extent in project settings will be used.
	 */
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(EditCondition="bProjectPointOnNavigationIfFailed"))
	FVector ProjectPointToNavigationExtent = FVector::ZeroVector;

	// Found random point
	UPROPERTY(EditAnywhere, Category="Output")
	FVector OutRandomPoint;
};

/**
 * Finds a random reachable point in a navigable space within a specified radius. Once the point is found, the Succeeded
 * status will be returned. Otherwise, if no point was found, the Failed status will be returned.
 */
USTRUCT(Category="AI", meta=(DisplayName="Get Random Reachable Point in Radius"))
struct FGetRandomReachablePointInRadiusStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FGetRandomReachablePointInRadiusStateTreeTask();

	using FInstanceDataType = FGetRandomReachablePointInRadiusStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};