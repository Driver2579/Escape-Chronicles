// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/ArrayContainsActorStateTreeCondition.h"

#include "StateTreeExecutionContext.h"

bool FArrayContainsActorStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_ENSURE
	ensureAlways(InstanceData.Actor);
#endif

	return InstanceData.Array.Contains(InstanceData.Actor) ^ bInvert;
}