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