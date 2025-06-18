// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "TeamIDsSet.generated.h"

class UAbilitySystemComponent;

/**
 * A struct that defines a pair of team tags and the corresponding team ID. Each actor that has all the tags from this
 * pair should be assigned to the team from this pair.
 */
USTRUCT(BlueprintType)
struct FTeamPair
{
	GENERATED_BODY()

	// If an actor has any of these tags, then he should be assigned to the team from this pair
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer TeamTags;

	// Team ID that should be assigned to actors with the tags from TeamTags
	UPROPERTY(EditAnywhere)
	FGenericTeamId TeamID;

	bool operator==(const FTeamPair& Other) const
	{
		return TeamID == Other.TeamID && TeamTags == Other.TeamTags;
	}
};

// This is required to use FTeamPair as a key in TMap and TSet
FORCEINLINE uint32 GetTypeHash(const FTeamPair& TeamPair)
{
	return GetTypeHash(TeamPair.TeamID);
}

// Non-mutable data asset that contains the team IDs for characters depending on their tags
UCLASS()
class ESCAPECHRONICLES_API UTeamIDsSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Finds the team ID for the given AbilitySystemComponent based on the tags defined in TeamPairs. If ASC doesn't
	 * match the tags of any team pair, the FGenericTeamId::NoTeam will be returned.
	 * @param AbilitySystemComponent Ability system component of an actor you want to get the team for.
	 * @return The team ID that should be assigned to the actor with the given AbilitySystemComponent.
	 */
	FGenericTeamId GetTeamForAbilitySystemComponent(const UAbilitySystemComponent* AbilitySystemComponent);

private:
	UPROPERTY(EditAnywhere)
	TSet<FTeamPair> TeamPairs;
};