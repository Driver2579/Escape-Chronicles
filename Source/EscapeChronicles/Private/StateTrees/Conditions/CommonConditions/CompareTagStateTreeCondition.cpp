// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/CompareTagStateTreeCondition.h"

#include "StateTreeExecutionContext.h"

bool FCompareTagStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const bool bTagMatches = InstanceData.bExactMatch ?
		InstanceData.Left.MatchesTagExact(InstanceData.Right) : InstanceData.Left.MatchesTag(InstanceData.Right);

	return bTagMatches ^ bInvert;
}