// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "ActivitySpotOccupationStateTreeEvaluator.generated.h"

class AActivitySpot;

USTRUCT()
struct FActivitySpotOccupationStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	// Activity Spot to listen for occupation changes on
	UPROPERTY(EditDefaultsOnly, Category="Context")
	TObjectPtr<AActivitySpot> ActivitySpot;

	// Whether the activity spot is currently occupied
	UPROPERTY(VisibleAnywhere, Category="Output")
	bool bOccupied = false;

	// Called when someone or something occupies the activity spot
	UPROPERTY(EditAnywhere)
	FStateTreeDelegateDispatcher OnActivitySpotOccupied;

	// Called when someone or something unoccupies the activity spot
	UPROPERTY(EditAnywhere)
	FStateTreeDelegateDispatcher OnActivitySpotUnoccupied;
};

// An evaluator that listens for the selected activity spot changes about the occupation state
USTRUCT(meta=(DisplayName="Activity Spot Occupation"))
struct FActivitySpotOccupationStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FActivitySpotOccupationStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};