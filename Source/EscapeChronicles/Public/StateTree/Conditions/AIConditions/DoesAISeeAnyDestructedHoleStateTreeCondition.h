// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "DoesAISeeAnyDestructedHoleStateTreeCondition.generated.h"

class AAIController;

USTRUCT()
struct FDoesAISeeAnyDestructedHoleStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// The AI controller that is checking the condition
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;

	// Actors the AI currently sees
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> SeenActors;
};

// Condition checking if AI sees any destructed hole in the given array of seen actors
USTRUCT(DisplayName="Does AI See Any Destructed Hole", Category="AI")
struct FDoesAISeeAnyDestructedHoleStateTreeCondition : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDoesAISeeAnyDestructedHoleStateTreeConditionInstanceData;

	FDoesAISeeAnyDestructedHoleStateTreeCondition() = default;

	explicit FDoesAISeeAnyDestructedHoleStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FDoesAISeeAnyDestructedHoleStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};