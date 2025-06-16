// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Evaluators/CommonEvaluators/CurrentEventStateTreeEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "GameState/EscapeChroniclesGameState.h"

void FCurrentEventStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	const UWorld* World = Context.GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return;
	}

	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	InstanceData.CachedGameState = World->GetGameState<AEscapeChroniclesGameState>();

	if (!ensureAlways(InstanceData.CachedGameState.IsValid()))
	{
		return;
	}

	// Initialize current events tags
	InstanceData.CurrentScheduledEventTag = InstanceData.CachedGameState->GetCurrentScheduledEventData().EventTag;
	InstanceData.CurrentActiveEventTag = InstanceData.CachedGameState->GetCurrentActiveEventData().EventTag;
}

void FCurrentEventStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// We should have a CachedGameState by now
	if (!ensureAlways(InstanceData.CachedGameState.IsValid()))
	{
		return;
	}

	// Get the current events tags to check if they changed
	const FGameplayTag CurrentScheduledEventTag = InstanceData.CachedGameState->GetCurrentScheduledEventData().EventTag;
	const FGameplayTag CurrentActiveEventTag = InstanceData.CachedGameState->GetCurrentActiveEventData().EventTag;

	// === Change the current events tags in instance data and broadcast the delegates if current events got changed ===

	if (CurrentScheduledEventTag != InstanceData.CurrentScheduledEventTag)
	{
		InstanceData.CurrentScheduledEventTag = CurrentScheduledEventTag;
		Context.BroadcastDelegate(InstanceData.OnCurrentScheduledEventChangedDispatcher);
	}

	if (CurrentActiveEventTag != InstanceData.CurrentActiveEventTag)
	{
		InstanceData.CurrentActiveEventTag = CurrentActiveEventTag;
		Context.BroadcastDelegate(InstanceData.OnCurrentActiveEventChangedDispatcher);
	}
}