// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "Common/Structs/ScheduleEventData.h"
#include "Tasks/StateTreeAITask.h"
#include "CurrentEventStateTreeEvaluator.generated.h"

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
};

/**
 * An evaluator that listens for current events that are set in the GameState. Once any type of current event is
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

	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;

private:
	mutable FDelegateHandle OnCurrentScheduledEventChangedDelegateHandle;
	mutable FDelegateHandle OnCurrentActiveEventChangedDelegateHandle;

	void OnCurrentScheduledEventChanged(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData) const;

	void OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData) const;

	/**
	 * I don't really like that we have to make these mutable, but it's the only way to update these from delegates
	 * instead of the tick. We still use the tick here, of course, but it's still more optimized to make it this way.
	 */
	mutable FGameplayTag CurrentScheduledEventTag;
	mutable FGameplayTag CurrentActiveEventTag;
};