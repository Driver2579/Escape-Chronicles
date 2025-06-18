// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "AbilitySystemComponentHasTagsStateTreeCondition.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FAbilitySystemComponentHasTagsStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Ability System Component to check for the tags
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// Tags to check in the Ability System Component
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTagContainer Tags;
};

// Condition checking if the given Ability System Component has the specified tags
USTRUCT(DisplayName="Ability System Component Has Tags", Category="Gameplay Tags")
struct FAbilitySystemComponentHasTagsStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAbilitySystemComponentHasTagsStateTreeConditionInstanceData;

	FAbilitySystemComponentHasTagsStateTreeCondition() = default;

	explicit FAbilitySystemComponentHasTagsStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FAbilitySystemComponentHasTagsStateTreeCondition(const bool bInInverts)
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