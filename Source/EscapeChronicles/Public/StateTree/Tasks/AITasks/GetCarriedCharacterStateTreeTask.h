// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "GetCarriedCharacterStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FGetCarriedCharacterStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// A character that carries another character
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AEscapeChroniclesCharacter> CarryingCharacter;

	// The nearest character to the controlled pawn
	UPROPERTY(VisibleAnywhere, Transient, Category="Output")
	TObjectPtr<AEscapeChroniclesCharacter> OutCarriedCharacter;
};

// Gets the carried character from the specified character. This task will fail if the character is not carrying anyone.
USTRUCT(DisplayName="Get Carried Character", Category="AI")
struct FGetCarriedCharacterStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FGetCarriedCharacterStateTreeTask();

	using FInstanceDataType = FGetCarriedCharacterStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};