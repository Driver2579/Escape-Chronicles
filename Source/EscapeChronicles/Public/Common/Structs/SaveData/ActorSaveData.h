// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Common/Structs/FunctionLibriries/MapFunctionLibriry.h"

#include "ActorSaveData.generated.h"

USTRUCT()
struct FSaveData
{
	GENERATED_BODY()

	// Transform of an actor or a scene component
	UPROPERTY()
	FTransform Transform;

	// Contains all properties of an actor or a component that are marked with "SaveGame"
	UPROPERTY()
	TArray<uint8> ByteData;

	bool operator==(const FSaveData& Other) const
	{
		return Transform.Equals(Other.Transform) && ByteData == Other.ByteData;
	}
};

USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

	// Save data of an actor itself
	UPROPERTY()
	FSaveData ActorSaveData;

	/**
	 * Save data for all actor's components.
	 * @tparam FName Name of the saved component.
	 * @tparam Save data for the associated component.
	 */
	UPROPERTY()
	TMap<FName, FSaveData> ComponentsSaveData;

	bool operator==(const FActorSaveData& Other) const
	{
		return ActorSaveData == Other.ActorSaveData &&
			FMapFunctionLibrary::AreMapsEqual(ComponentsSaveData, Other.ComponentsSaveData);
	}
};