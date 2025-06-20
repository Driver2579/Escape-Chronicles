// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Evaluators/CommonEvaluators/CharacterStatusStateTreeEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Structs/CharacterStatusEvents.h"

void FCharacterStatusStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ClearEvaluatorData(InstanceData);

	// Subscribe to the character fainted status changed event to know when someone has fainted or revived
	InstanceData.OnCharacterFaintedStatusChangedDelegateHandle =
		FCharacterStatusEvents::OnFaintedStatusChanged.AddRaw(this,
			&FCharacterStatusStateTreeEvaluator::OnCharacterFaintedStatusChanged);
}

void FCharacterStatusStateTreeEvaluator::OnCharacterFaintedStatusChanged(AEscapeChroniclesCharacter* Character,
	const bool bNewStatus) const
{
#if DO_ENSURE
	ensureAlways(IsValid(Character));
#endif

	// Construct the event payload to send it to the StateTree in the next tick
	LastCharacterFaintedStatusChangedPayload = FOnCharacterStatusChangedStateTreeEventPayload(Character, bNewStatus);

	// Clear the list of instances that sent the last payload to send the new event in the next tick
	InstancesThatSentLastCharacterFaintedStatusChangedPayload.Empty();
}

void FCharacterStatusStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// Wait for the next tick if the payload is not valid yet
	if (!LastCharacterFaintedStatusChangedPayload.IsValid())
	{
		return;
	}

	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	/**
	 * Return if the last character fainted status changed event for this node instance was already sent in the previous
	 * tick.
	 */
	if (InstancesThatSentLastCharacterFaintedStatusChangedPayload.Contains(&InstanceData))
	{
		return;
	}

#if DO_ENSURE
	ensureAlways(InstanceData.OnCharacterFaintedStatusChangedEventTag.IsValid());
#endif

	// Send an event with a valid payload to the StateTree
	Context.SendEvent(InstanceData.OnCharacterFaintedStatusChangedEventTag,
		FConstStructView::Make(LastCharacterFaintedStatusChangedPayload));

	/**
	 * Remember that this node instance has sent an event to avoid sending it again in the next tick until the
	 * OnCharacterFaintedStatusChanged function is called again.
	 */
	InstancesThatSentLastCharacterFaintedStatusChangedPayload.Add(&InstanceData);
}

void FCharacterStatusStateTreeEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	ClearEvaluatorData(Context.GetInstanceData(*this));
}

void FCharacterStatusStateTreeEvaluator::ClearEvaluatorData(FInstanceDataType& InstanceData) const
{
	// Reset the parameters
	LastCharacterFaintedStatusChangedPayload.Reset();
	InstancesThatSentLastCharacterFaintedStatusChangedPayload.Empty();

	// Unsubscribe from the event once the evaluator is stopped to avoid crashes
	FCharacterStatusEvents::OnFaintedStatusChanged.Remove(InstanceData.OnCharacterFaintedStatusChangedDelegateHandle);
	InstanceData.OnCharacterFaintedStatusChangedDelegateHandle.Reset();
}