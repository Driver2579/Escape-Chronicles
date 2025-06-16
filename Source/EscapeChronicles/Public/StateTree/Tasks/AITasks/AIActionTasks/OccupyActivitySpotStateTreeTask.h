// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "OccupyActivitySpotStateTreeTask.generated.h"

class AActivitySpot;
class AEscapeChroniclesCharacter;

USTRUCT()
struct FOccupyActivitySpotStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Character that will occupy the activity spot
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AEscapeChroniclesCharacter> Character;

	// Activity spot to occupy
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActivitySpot> ActivitySpot;

	// If true, the character will unoccupy the activity spot when exiting the task
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bUnoccupyOnExit = false;
};

/**
 * A task that allows a character to occupy an activity spot. Optionally, the character will unoccupy the spot when
 * exiting the task.
 */
USTRUCT(Category="AI|Action", meta=(DisplayName="Occupy Activity Spot"))
struct FOccupyActivitySpotStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FOccupyActivitySpotStateTreeTask();

	using FInstanceDataType = FOccupyActivitySpotStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};