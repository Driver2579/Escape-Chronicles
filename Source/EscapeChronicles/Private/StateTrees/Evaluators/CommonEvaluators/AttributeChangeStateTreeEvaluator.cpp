// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Evaluators/CommonEvaluators/AttributeChangeStateTreeEvaluator.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"

void FAttributeChangeStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Actor);
#endif

#if DO_ENSURE
	ensureAlways(InstanceData.Attribute.IsValid());
#endif

	// Cache the ASC to not search for it every tick
	InstanceData.AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		InstanceData.Actor);

	if (!ensureAlways(InstanceData.AbilitySystemComponent.IsValid()))
	{
		return;
	}

#if DO_ENSURE
	ensureAlwaysMsgf(InstanceData.AbilitySystemComponent->HasAttributeSetForAttribute(InstanceData.Attribute),
		TEXT("An ability system component of the given actor %s doesn't have an attribute %s!"),
		*InstanceData.Actor->GetName(), *InstanceData.Attribute.GetName());
#endif

	// Initialize the values of the attribute
	InstanceData.BaseValue = InstanceData.AbilitySystemComponent->GetNumericAttributeBase(InstanceData.Attribute);
	InstanceData.CurrentValue = InstanceData.AbilitySystemComponent->GetNumericAttribute(InstanceData.Attribute);
}

void FAttributeChangeStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!InstanceData.AbilitySystemComponent.IsValid())
	{
		return;
	}

	// Get the current values of the attribute
	const float BaseValue = InstanceData.AbilitySystemComponent->GetNumericAttributeBase(InstanceData.Attribute);
	const float CurrentValue = InstanceData.AbilitySystemComponent->GetNumericAttribute(InstanceData.Attribute);

	// Remember the new value and broadcast the delegate if the base value has changed
	if (InstanceData.BaseValue != BaseValue)
	{
		InstanceData.BaseValue = BaseValue;
		Context.BroadcastDelegate(InstanceData.OnAttributeBaseValueChangedDispatcher);
	}

	// Remember the new value and broadcast the delegate if the current value has changed
	if (InstanceData.CurrentValue != CurrentValue)
	{
		InstanceData.CurrentValue = CurrentValue;
		Context.BroadcastDelegate(InstanceData.OnAttributeCurrentValueChangedDispatcher);
	}
}