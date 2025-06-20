// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/DataAssets/TeamIDsSet.h"

#include "AbilitySystemComponent.h"

FGenericTeamId UTeamIDsSet::GetTeamForAbilitySystemComponent(const UAbilitySystemComponent* AbilitySystemComponent)
{
#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Find the team pair that matches the tags of the AbilitySystemComponent
	for (const FTeamPair& TeamPair : TeamPairs)
	{
		if (AbilitySystemComponent->HasAllMatchingGameplayTags(TeamPair.TeamTags))
		{
			return TeamPair.TeamID;
		}
	}

	// Return NoTeam if no matching team pair was found
	return FGenericTeamId::NoTeam;
}