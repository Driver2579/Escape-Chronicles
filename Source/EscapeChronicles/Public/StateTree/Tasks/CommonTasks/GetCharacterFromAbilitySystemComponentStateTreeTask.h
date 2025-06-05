// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetCharacterFromAbilitySystemComponentStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;
class UAbilitySystemComponent;

USTRUCT()
struct FGetCharacterFromAbilitySystemComponentStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Ability System Component to get the character from
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere, Category="Output")
	TObjectPtr<AEscapeChroniclesCharacter> OutCharacter;
};

/**
 * Gets the character from the given Ability System Component. The character must be an AvatarActor of the Ability
 * System Component or the task will fail.
 */
USTRUCT(meta=(DisplayName="Get Character From Ability System Component"))
struct FGetCharacterFromAbilitySystemComponentStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetCharacterFromAbilitySystemComponentStateTreeTask();

	using FInstanceDataType = FGetCharacterFromAbilitySystemComponentStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};