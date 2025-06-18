// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "DoesCharacterOwnASCStateTreeCondition.generated.h"

class AEscapeChroniclesCharacter;
class UAbilitySystemComponent;

USTRUCT()
struct FDoesCharacterOwnASCStateTreeConditionInstanceData
{
	GENERATED_BODY()

	// Character to check whether it owns the Ability System Component
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AEscapeChroniclesCharacter> Character;

	// Ability System Component to check whether it is owned by the given character
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};

// Condition checking if the given character is the owner of the given Ability System Component
USTRUCT(DisplayName="Does Character Own Ability System Component")
struct FDoesCharacterOwnASCStateTreeCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDoesCharacterOwnASCStateTreeConditionInstanceData;

	FDoesCharacterOwnASCStateTreeCondition() = default;

	explicit FDoesCharacterOwnASCStateTreeCondition(const EStateTreeCompare InInverts)
		: bInvert(InInverts == EStateTreeCompare::Invert)
	{}

	explicit FDoesCharacterOwnASCStateTreeCondition(const bool bInInverts)
		: bInvert(bInInverts)
	{}

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};