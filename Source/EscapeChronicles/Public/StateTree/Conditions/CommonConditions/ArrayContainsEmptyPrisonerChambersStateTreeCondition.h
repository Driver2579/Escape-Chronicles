// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "ArrayContainsEmptyPrisonerChambersStateTreeCondition.generated.h"

USTRUCT()
struct FArrayContainsEmptyPrisonerChambersStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Actors to check for prisoner chambers without their owning characters in it
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> ActorsToCheck;
};

/**
 * Condition checking if an array of actors contains any prisoner chambers that have owning characters that are
 * currently in the game but aren't in their chambers, OR if that prisoner chamber has a bed that has the same owning
 * character and this bed isn't empty (someone's sleeping in it).
 */
USTRUCT(DisplayName="Array Contains Empty Prisoner Chambers")
struct FArrayContainsEmptyPrisonerChambersStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FArrayContainsEmptyPrisonerChambersStateTreeConditionInstanceData;

	FArrayContainsEmptyPrisonerChambersStateTreeCondition() = default;

	explicit FArrayContainsEmptyPrisonerChambersStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FArrayContainsEmptyPrisonerChambersStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};