// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/AbilitySystemComponentHasTagsStateTreeCondition.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"

bool FAbilitySystemComponentHasTagsStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (InstanceData.AbilitySystemComponent)
	{
		// Check if the Ability System Component has all the specified tags and invert the result if needed
		return InstanceData.AbilitySystemComponent->HasAllMatchingGameplayTags(InstanceData.Tags) ^ bInvert;
	}

	// Return false if the Ability System Component is not valid. If bInvert is true, then true will be returned.
	return bInvert;
}