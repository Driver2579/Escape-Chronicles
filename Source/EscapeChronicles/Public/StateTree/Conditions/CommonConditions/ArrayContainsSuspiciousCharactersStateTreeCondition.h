// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Tasks/StateTreeAITask.h"
#include "ArrayContainsSuspiciousCharactersStateTreeCondition.generated.h"

USTRUCT()
struct FArrayContainsSuspiciousCharactersStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Actors to check for suspicious characters
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> ActorsToCheck;

	// The minimum suspicious value to consider a character suspicious
	UPROPERTY(EditAnywhere, Category="Parameter")
	float MinSuspicious = 90;
};

/**
 * Condition checking if an array of actors contains any characters with a suspicion attribute value above a specified
 * one.
 */
USTRUCT(DisplayName="Array Contains Suspicious Characters")
struct FArrayContainsSuspiciousCharactersStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FArrayContainsSuspiciousCharactersStateTreeConditionInstanceData;

	FArrayContainsSuspiciousCharactersStateTreeCondition() = default;

	explicit FArrayContainsSuspiciousCharactersStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FArrayContainsSuspiciousCharactersStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};