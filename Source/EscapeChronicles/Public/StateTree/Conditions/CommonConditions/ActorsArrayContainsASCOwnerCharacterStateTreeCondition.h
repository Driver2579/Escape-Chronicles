// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "ActorsArrayContainsASCOwnerCharacterStateTreeCondition.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FActorsArrayContainsASCOwnerCharacterStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Array to check if it contains a character
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> Array;

	// Ability System Component to get the character from
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};

// Condition checking if an array of actors contains a specific character that owns a specific Ability System Component
USTRUCT(DisplayName="Actors Array Contains Ability System Component's Owner Character")
struct FActorsArrayContainsASCOwnerCharacterStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FActorsArrayContainsASCOwnerCharacterStateTreeConditionInstanceData;

	FActorsArrayContainsASCOwnerCharacterStateTreeCondition() = default;

	explicit FActorsArrayContainsASCOwnerCharacterStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FActorsArrayContainsASCOwnerCharacterStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};