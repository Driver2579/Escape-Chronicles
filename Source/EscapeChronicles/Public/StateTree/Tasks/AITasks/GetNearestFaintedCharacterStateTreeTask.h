// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "GetNearestFaintedCharacterStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FGetNearestFaintedCharacterStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// A pawn controlled by the AIController
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// The nearest fainted character to the controlled pawn
	UPROPERTY(VisibleAnywhere, Transient, Category="Output")
	TObjectPtr<AEscapeChroniclesCharacter> OutNearestFaintedCharacter;
};

// Finds the nearest fainted character in the world to the AI pawn
USTRUCT(DisplayName="Get Nearest Fainted Character", Category="AI")
struct FGetNearestFaintedCharacterStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FGetNearestFaintedCharacterStateTreeTask();

	using FInstanceDataType = FGetNearestFaintedCharacterStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};