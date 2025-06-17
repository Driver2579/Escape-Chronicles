// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/ArrayContainsActorsMatchingTagQueryStateTreeCondition.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "StateTreeExecutionContext.h"

bool FArrayContainsActorsMatchingTagQueryStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Find the first actor in the array that matches the tag query
	for (const AActor* Actor : InstanceData.Array)
	{
		const UAbilitySystemComponent* AbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);

		const bool bActorMatchesTagQuery = IsValid(AbilitySystemComponent) &&
			AbilitySystemComponent->MatchesGameplayTagQuery(InstanceData.TagQuery);

		// Return true if an actor matches the given tag query. If bInvert is true, then false will be returned.
		if (bActorMatchesTagQuery)
		{
			return !bInvert;
		}
	}

	// Return false if no actors matched the tag query. If bInvert is true, then true will be returned.
	return bInvert;
}