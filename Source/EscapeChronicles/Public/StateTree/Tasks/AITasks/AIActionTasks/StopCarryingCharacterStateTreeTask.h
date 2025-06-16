// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "StopCarryingCharacterStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FStopCarryingCharacterStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Character that will stop carrying the character if any
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AEscapeChroniclesCharacter> CarryingCharacter;
};

// A task that allows a character to stop carrying the currently carried character if any
USTRUCT(Category="AI|Action", meta=(DisplayName="Stop Carrying the Character"))
struct FStopCarryingCharacterStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FStopCarryingCharacterStateTreeTask();

	using FInstanceDataType = FStopCarryingCharacterStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};