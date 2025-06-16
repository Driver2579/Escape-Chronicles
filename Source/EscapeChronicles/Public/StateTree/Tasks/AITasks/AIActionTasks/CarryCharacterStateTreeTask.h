// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "CarryCharacterStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FCarryCharacterStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Character that will carry another character
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AEscapeChroniclesCharacter> CarryingCharacter;

	// Character to carry
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AEscapeChroniclesCharacter> TargetCharacter;

	// If true, the character will stop carrying another character when exiting the task
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bStopCarryingOnExit = false;
};

// A task that allows a character to carry another character. Optionally stops carrying when exiting the task.
USTRUCT(Category="AI|Action", meta=(DisplayName="Carry Character"))
struct FCarryCharacterStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FCarryCharacterStateTreeTask();

	using FInstanceDataType = FCarryCharacterStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};