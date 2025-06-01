// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "ArrayContainsActorStateTreeCondition.generated.h"

USTRUCT()
struct FArrayContainsActorStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Array to check if it contains an actor
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> Array;

	// Actor to check if it's in the array
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActor> Actor;
};

// Condition checking if an array of actors contains a specific actor
USTRUCT(DisplayName="Array Contains Actor")
struct FArrayContainsActorStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FArrayContainsActorStateTreeConditionInstanceData;

	FArrayContainsActorStateTreeCondition() = default;

	explicit FArrayContainsActorStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FArrayContainsActorStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};