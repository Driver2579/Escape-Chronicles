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

	AEscapeChroniclesGameState* GameState = World->GetGameState<AEscapeChroniclesGameState>();

	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	// === Listen for current events to be changed ===

	OnCurrentScheduledEventChangedDelegateHandle = GameState->OnCurrentScheduledEventChanged.AddRaw(this,
		&FCurrentEventStateTreeEvaluator::OnCurrentScheduledEventChanged);

	OnCurrentActiveEventChangedDelegateHandle = GameState->OnCurrentActiveEventChanged.AddRaw(this,
		&FCurrentEventStateTreeEvaluator::OnCurrentActiveEventChanged);
}

void FCurrentEventStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// === Change the current events tags in instance data and broadcast the delegates if current events got changed ===

	if (CurrentScheduledEventTag != InstanceData.CurrentScheduledEventTag)
	{
		InstanceData.CurrentScheduledEventTag = CurrentScheduledEventTag;
		Context.BroadcastDelegate(InstanceData.CurrentScheduledEventChangedDispatcher);
	}

	if (CurrentActiveEventTag != InstanceData.CurrentActiveEventTag)
	{
		InstanceData.CurrentActiveEventTag = CurrentActiveEventTag;
		Context.BroadcastDelegate(InstanceData.CurrentActiveEventChangedDispatcher);
	}
}

void FCurrentEventStateTreeEvaluator::OnCurrentScheduledEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData) const
{
	CurrentScheduledEventTag = NewEventData.EventTag;
}

void FCurrentEventStateTreeEvaluator::OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData) const
{
	CurrentActiveEventTag = NewEventData.EventTag;
}

void FCurrentEventStateTreeEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	const UWorld* World = Context.GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	AEscapeChroniclesGameState* GameState = World->GetGameState<AEscapeChroniclesGameState>();

	// Unsubscribe from the delegates
	if (IsValid(GameState))
	{
		GameState->OnCurrentScheduledEventChanged.Remove(OnCurrentScheduledEventChangedDelegateHandle);
		GameState->OnCurrentActiveEventChanged.Remove(OnCurrentActiveEventChangedDelegateHandle);
	}
}