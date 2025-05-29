// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "Common/Structs/ScheduleEventData.h"
#include "Tasks/StateTreeAITask.h"
#include "CurrentEventStateTreeEvaluator.generated.h"

class AEscapeChroniclesGameState;

USTRUCT()
struct FCurrentEventStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	// An event that is currently scheduled to be active but could be paused due to being overriden by another event
	UPROPERTY(EditAnywhere, Category="Output")
	FGameplayTag CurrentScheduledEventTag;

	// An event that is currently active
	UPROPERTY(EditAnywhere, Category="Output")
	FGameplayTag CurrentActiveEventTag;

	// A delegate that is called when the CurrentScheduledEventTag changes
	UPROPERTY(EditAnywhere)
	FStateTreeDelegateDispatcher OnCurrentScheduledEventChangedDispatcher;

	// A delegate that is called when the CurrentActiveEventTag changes
	UPROPERTY(EditAnywhere)
	FStateTreeDelegateDispatcher OnCurrentActiveEventChangedDispatcher;

	TWeakObjectPtr<AEscapeChroniclesGameState> CachedGameState;
};

/**
 * An evaluator that listens for current events that are set in the CachedGameState. Once any type of current event is
 * changed, the delegate is broadcast.
 */
USTRUCT(meta=(DisplayName="Current Event"))
struct FCurrentEventStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCurrentEventStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};