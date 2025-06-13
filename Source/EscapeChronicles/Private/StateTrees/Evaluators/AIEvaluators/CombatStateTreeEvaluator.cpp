// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Evaluators/AIEvaluators/CombatStateTreeEvaluator.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Common/Structs/CombatEvents.h"

void FCombatStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ClearEvaluatorData(InstanceData);

	// Subscribe to the punch hit event to know when someone performs a punch to some target
	InstanceData.OnPunchHitDelegateHandle = FCombatEvents::OnPunchHit.AddRaw(this,
		&FCombatStateTreeEvaluator::OnPunchHit);
}

void FCombatStateTreeEvaluator::OnPunchHit(UAbilitySystemComponent* Instigator, UAbilitySystemComponent* Target,
	FActiveGameplayEffectHandle TargetAppliedEffectHandle) const
{
#if DO_ENSURE
	ensureAlways(IsValid(Instigator));
	ensureAlways(IsValid(Target));
#endif

	// Construct the event payload to send it to the StateTree in the next tick
	LastPunchHitPayload = FOnPunchHitStateTreeEventPayload(Instigator, Target, TargetAppliedEffectHandle);

	// Clear the list of instances that sent the last payload to send the new event in the next tick
	InstancesThatSentLastPunchHitPayload.Empty();
}

void FCombatStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// Wait for the next tick if the payload is not valid yet
	if (!LastPunchHitPayload.IsValid())
	{
		return;
	}

	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Return if the last punch event for this node instance was already sent in the previous tick
	if (InstancesThatSentLastPunchHitPayload.Contains(&InstanceData))
	{
		return;
	}

#if DO_ENSURE
	ensureAlways(InstanceData.OnPunchHitEventTag.IsValid());
#endif

	// Send an event with a valid payload to the StateTree
	Context.SendEvent(InstanceData.OnPunchHitEventTag, FConstStructView::Make(LastPunchHitPayload));

	/**
	 * Remember that this node instance has sent an event to avoid sending it again in the next tick until the
	 * OnPunchHit function is called again.
	 */
	InstancesThatSentLastPunchHitPayload.Add(&InstanceData);
}

void FCombatStateTreeEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	ClearEvaluatorData(Context.GetInstanceData(*this));
}

void FCombatStateTreeEvaluator::ClearEvaluatorData(FInstanceDataType& InstanceData) const
{
	// Reset the parameters
	LastPunchHitPayload.Reset();
	InstancesThatSentLastPunchHitPayload.Empty();

	// Unsubscribe from the event once the evaluator is stopped to avoid crashes
	FCombatEvents::OnPunchHit.Remove(InstanceData.OnPunchHitDelegateHandle);
	InstanceData.OnPunchHitDelegateHandle.Reset();
}