// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/ListenForAttributeChangeStateTreeTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FListenForAttributeChangeStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_ENSURE
	ensureAlways(InstanceData.Actor);
	ensureAlways(InstanceData.Attribute.IsValid());
#endif

	// Cache the ASC to not search for it every tick
	InstanceData.AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		InstanceData.Actor);

	if (!ensureAlways(InstanceData.AbilitySystemComponent.IsValid()))
	{
		return EStateTreeRunStatus::Failed;
	}

#if DO_CHECK
	checkf(InstanceData.AbilitySystemComponent->HasAttributeSetForAttribute(InstanceData.Attribute),
		TEXT("An ability system component of the given actor %s doesn't have an attribute %s!"),
		*InstanceData.Actor->GetName(), *InstanceData.Attribute.GetName());
#endif

	// Initialize the values of the attribute
	InstanceData.BaseValue = InstanceData.AbilitySystemComponent->GetNumericAttributeBase(InstanceData.Attribute);
	InstanceData.CurrentValue = InstanceData.AbilitySystemComponent->GetNumericAttribute(InstanceData.Attribute);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FListenForAttributeChangeStateTreeTask::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!ensureAlways(InstanceData.AbilitySystemComponent.IsValid()))
	{
		return EStateTreeRunStatus::Failed;
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

	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FListenForAttributeChangeStateTreeTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();

#if DO_CHECK
	check(InstanceData);
#endif

	return FText::FromString(
		FString::Printf(TEXT("Listen For Attribute %s Change"), *InstanceData->Attribute.GetName()));
}
#endif