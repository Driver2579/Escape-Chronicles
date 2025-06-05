// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "ClearFocusStateTreeTask.generated.h"

class AAIController;

USTRUCT()
struct FClearFocusStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// The AI controller of the bot that the focus should be cleared on
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;
};

// Clears the focus of the bot
USTRUCT(DisplayName="Clear Focus", Category="AI|Action")
struct FClearFocusStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FClearFocusStateTreeTask();

	using FInstanceDataType = FClearFocusStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};