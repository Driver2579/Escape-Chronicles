// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttributeSetSaveData.generated.h"

struct FGameplayAttribute;

USTRUCT()
struct FAttributeSetSaveData
{
	GENERATED_BODY()

	/**
	 * @tparam KeyType The name of the attribute.
	 * @tparam ValueType The base value of the attribute.
	 */
	UPROPERTY(SaveGame)
	TMap<FString, float> AttributesBaseValues;
};