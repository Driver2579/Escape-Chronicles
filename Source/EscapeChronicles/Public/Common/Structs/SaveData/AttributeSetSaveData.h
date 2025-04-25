// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttributeSetSaveData.generated.h"

struct FGameplayAttribute;

USTRUCT()
struct FAttributeSetSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FGameplayAttribute, float> AttributesBaseValues;
};