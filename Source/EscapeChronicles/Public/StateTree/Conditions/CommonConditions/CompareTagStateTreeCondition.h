// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "CompareTagStateTreeCondition.generated.h"

USTRUCT()
struct FCompareTagStateTreeConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input")
	FGameplayTag Left;

	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTag Right;

	// If true, the MatchesTagExact function will be used instead of the MatchesTag function when comparing tags
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bExactMatch = false;
};

// Condition comparing two tags
USTRUCT(DisplayName="Tag Compare", Category="Gameplay Tags")
struct FCompareTagStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCompareTagStateTreeConditionInstanceData;

	FCompareTagStateTreeCondition() = default;

	explicit FCompareTagStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FCompareTagStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	virtual FName GetIconName() const override
	{
		return FName("StateTreeEditorStyle|Node.Tag");
	}
#endif

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};