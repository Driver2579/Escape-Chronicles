// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "ArrayContainsPrisonersOutsideTheirChambersStateTreeCondition.generated.h"

USTRUCT()
struct FArrayContainsPrisonersOutsideTheirChambersStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Actors to check for characters outside their chambers
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> ActorsToCheck;

	// If the character has at least one of these tags, then he will not be considered as the one outside his chamber
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTagContainer AllowedGameplayTags;
};

/**
 * Condition checking if an array of actors contains any characters that own any prisoner chambers and aren't currently
 * in any of them.
 */
USTRUCT(DisplayName="Array Contains Prisoners Outside Their Chambers")
struct FArrayContainsPrisonersOutsideTheirChambersStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FArrayContainsPrisonersOutsideTheirChambersStateTreeConditionInstanceData;

	FArrayContainsPrisonersOutsideTheirChambersStateTreeCondition() = default;

	explicit FArrayContainsPrisonersOutsideTheirChambersStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FArrayContainsPrisonersOutsideTheirChambersStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};