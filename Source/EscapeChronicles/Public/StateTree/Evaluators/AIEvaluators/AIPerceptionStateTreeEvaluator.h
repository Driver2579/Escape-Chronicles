// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AIPerceptionStateTreeEvaluator.generated.h"

class AAIController;

USTRUCT()
struct FAISeesCharacterStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	// An AI-controlled pawn with an AI perception component on the AI controller that we want to watch
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> Pawn;

	// Actors that the AI currently sees
	UPROPERTY(EditAnywhere, Category="Output")
	TArray<TObjectPtr<AActor>> SeenActors;

	// A delegate that is called when the SeenActors array changes
	UPROPERTY(EditAnywhere)
	FStateTreeDelegateDispatcher OnSeenActorsChangedDispatcher;

	TWeakObjectPtr<AAIController> AIController;
};

// An evaluator that gets values from AI perception
USTRUCT(Category="AI", meta=(DisplayName="AI Perception"))
struct FAIPerceptionStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAISeesCharacterStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};