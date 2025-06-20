// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "ArrayContainsActorsMatchingTagQueryStateTreeCondition.generated.h"

USTRUCT()
struct FArrayContainsActorsMatchingTagQueryStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Array to check if it contains any actors that match the specified tag query
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> Array;

	// Tag query to check against the actors in the array
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTagQuery TagQuery;
};

// Condition checking if an array of actors contains any actors that match a specified tag query.
USTRUCT(DisplayName="Array Contains Actors Matching Tag Query", Category="Gameplay Tags")
struct FArrayContainsActorsMatchingTagQueryStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FArrayContainsActorsMatchingTagQueryStateTreeConditionInstanceData;

	FArrayContainsActorsMatchingTagQueryStateTreeCondition() = default;

	explicit FArrayContainsActorsMatchingTagQueryStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FArrayContainsActorsMatchingTagQueryStateTreeCondition(const bool bInInverts)
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

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
};