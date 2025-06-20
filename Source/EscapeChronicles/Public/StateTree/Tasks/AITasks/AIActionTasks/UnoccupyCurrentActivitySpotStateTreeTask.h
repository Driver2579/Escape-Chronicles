// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "UnoccupyCurrentActivitySpotStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FUnoccupyCurrentActivitySpotStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Character that is potentially occupying the activity spot
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AEscapeChroniclesCharacter> Character;
};

/**
 * A task that allows a character to unoccupy the currently occupied activity spot. Returns Failed if the character
 * doesn't occupy any.
 */
USTRUCT(Category="AI|Action", meta=(DisplayName="Unoccupy Current Activity Spot"))
struct FUnoccupyCurrentActivitySpotStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FUnoccupyCurrentActivitySpotStateTreeTask();

	using FInstanceDataType = FUnoccupyCurrentActivitySpotStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};