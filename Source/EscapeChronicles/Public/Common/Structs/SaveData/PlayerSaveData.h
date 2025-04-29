// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ActorSaveData.h"
#include "Common/Structs/FunctionLibriries/MapFunctionLibriry.h"
#include "PlayerSaveData.generated.h"

USTRUCT()
struct FPlayerSaveData
{
	GENERATED_BODY()

	/**
	 * @tparam KeyType Class of the actor that exists with the player (e.g., Pawn, PlayerState, PlayerController, etc.).
	 * @tparam ValueType Save data for the associated class.
	 */
	UPROPERTY()
	TMap<TSoftClassPtr<AActor>, FActorSaveData> PlayerSpecificActorsSaveData;

	bool operator==(const FPlayerSaveData& Other) const
	{
		return FMapFunctionLibrary::AreMapsEqual(PlayerSpecificActorsSaveData,
			Other.PlayerSpecificActorsSaveData);
	}
};