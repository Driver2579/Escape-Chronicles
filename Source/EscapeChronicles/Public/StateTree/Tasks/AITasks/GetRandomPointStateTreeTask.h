// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "GetRandomPointStateTreeTask.generated.h"

class AAIController;

USTRUCT()
struct FGetRandomPointStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;

	// A pawn controlled by the AIController.
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	/**
	 * If true, the task will find a random reachable point	withing the specified radius. Otherwise, it will find a
	 * random point in all navigable space.
	 */
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bUseRadius = true;

	// The radius within the random reachable point will be searched
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0.1, EditCondition="bUseRadius"))
	float Radius = 1000;

	// Filter to use when searching for points. Can be left empty if you don't need any specific filtering.
	UPROPERTY(EditAnywhere, Category="Parameter")
	TSubclassOf<class UNavigationQueryFilter> QueryFilterClass;

	// Found random point
	UPROPERTY(EditAnywhere, Category="Output")
	FVector OutRandomPoint;
};

/**
 * Finds a random point in navigable space either within a specified radius or anywhere in the navigable area. Once the
 * point is found, the Succeeded status will be returned. Otherwise, if no point was found, the Failed status will be
 * returned.
 */
USTRUCT(Category="AI", meta=(DisplayName="Get Random Reachable Point in Radius"))
struct FGetRandomPointStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FGetRandomPointStateTreeTask();

	using FInstanceDataType = FGetRandomPointStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};