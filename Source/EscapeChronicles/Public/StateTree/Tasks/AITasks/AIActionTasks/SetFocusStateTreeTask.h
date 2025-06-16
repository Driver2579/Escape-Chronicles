// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "SetFocusStateTreeTask.generated.h"

class AAIController;

USTRUCT()
struct FSetFocusStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// The AI controller of the bot that the focus will be set on
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;

	// An actor that the bot will focus on
	UPROPERTY(EditAnywhere, Category="Input")
	AActor* FocusActor;

	// Whether the focus should be cleared when the task ends
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bClearFocusOnTaskExit = false;
};

// Sets the focus of the bot to a specified actor
USTRUCT(DisplayName="Set Focus", Category="AI|Action")
struct FSetFocusStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FSetFocusStateTreeTask();

	using FInstanceDataType = FSetFocusStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};