// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "GetNearestCharacterStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FGetNearestCharacterStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// A pawn controlled by the AIController
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// Characters to find the nearest one to the controlled pawn in
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> Characters;

	// The nearest character to the controlled pawn
	UPROPERTY(EditAnywhere, Category="Output")
	TObjectPtr<AEscapeChroniclesCharacter> OutNearestCharacter;
};

// Finds the nearest character to the AI pawn from the given list of characters
USTRUCT(Category="AI", meta=(DisplayName="Get Nearest Character"))
struct FGetNearestCharacterStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FGetNearestCharacterStateTreeTask();

	using FInstanceDataType = FGetNearestCharacterStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};