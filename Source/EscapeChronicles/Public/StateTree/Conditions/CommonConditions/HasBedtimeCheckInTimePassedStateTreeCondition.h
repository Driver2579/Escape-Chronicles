// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "HasBedtimeCheckInTimePassedStateTreeCondition.generated.h"

USTRUCT()
struct FHasBedtimeCheckInTimePassedStateTreeConditionInstanceData
{
	GENERATED_BODY()
};

/**
 * Condition checking if the bedtime event is currently active and if its limited time for players to check in has
 * passed.
 */
USTRUCT(DisplayName="Has Bedtime Check in Time Passed")
struct FHasBedtimeCheckInTimePassedStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FHasBedtimeCheckInTimePassedStateTreeConditionInstanceData;

	FHasBedtimeCheckInTimePassedStateTreeCondition() = default;

	explicit FHasBedtimeCheckInTimePassedStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FHasBedtimeCheckInTimePassedStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};