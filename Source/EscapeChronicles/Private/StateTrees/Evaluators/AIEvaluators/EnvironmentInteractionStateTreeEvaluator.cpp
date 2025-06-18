// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Evaluators/AIEvaluators/EnvironmentInteractionStateTreeEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "Common/Structs/EnvironmentInteractionEvents.h"

void FEnvironmentInteractionStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ClearEvaluatorData(InstanceData);

	// Subscribe to the destructible damaged event to know when someone damages a destructible component
	InstanceData.OnDestructibleDamagedDelegateHandle = FEnvironmentInteractionEvents::OnDestructibleDamaged.AddRaw(
		this, &FEnvironmentInteractionStateTreeEvaluator::OnDestructibleDamaged);
}

void FEnvironmentInteractionStateTreeEvaluator::OnDestructibleDamaged(AEscapeChroniclesCharacter* Character,
	UDestructibleComponent* DestructibleComponent) const
{
#if DO_CHECK
	check(IsValid(Character));
	check(IsValid(DestructibleComponent));
#endif

	// Construct the event payload to send it to the StateTree in the next tick
	LastDestructibleHitPayload = FOnDestructibleDamagedStateTreeEventPayload(Character, DestructibleComponent);

	// Clear the list of instances that sent the last payload to send the new event in the next tick
	InstancesThatSentLastDestructibleDamagedPayload.Empty();
}

void FEnvironmentInteractionStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// Wait for the next tick if the payload is not valid yet
	if (!LastDestructibleHitPayload.IsValid())
	{
		return;
	}

	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Return if the last destructible damaged event for this node instance was already sent in the previous tick
	if (InstancesThatSentLastDestructibleDamagedPayload.Contains(&InstanceData))
	{
		return;
	}

#if DO_ENSURE
	ensureAlways(InstanceData.OnDestructibleDamagedEventTag.IsValid());
#endif

	// Send an event with a valid payload to the StateTree
	Context.SendEvent(InstanceData.OnDestructibleDamagedEventTag,
		FConstStructView::Make(LastDestructibleHitPayload));

	/**
	 * Remember that this node instance has sent an event to avoid sending it again in the next tick until the
	 * OnDestructibleDamaged function is called again.
	 */
	InstancesThatSentLastDestructibleDamagedPayload.Add(&InstanceData);
}

void FEnvironmentInteractionStateTreeEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	ClearEvaluatorData(Context.GetInstanceData(*this));
}

void FEnvironmentInteractionStateTreeEvaluator::ClearEvaluatorData(FInstanceDataType& InstanceData) const
{
	// Reset the parameters
	LastDestructibleHitPayload.Reset();
	InstancesThatSentLastDestructibleDamagedPayload.Empty();

	// Unsubscribe from the event once the evaluator is stopped to avoid crashes
	FEnvironmentInteractionEvents::OnDestructibleDamaged.Remove(InstanceData.OnDestructibleDamagedDelegateHandle);
	InstanceData.OnDestructibleDamagedDelegateHandle.Reset();
}