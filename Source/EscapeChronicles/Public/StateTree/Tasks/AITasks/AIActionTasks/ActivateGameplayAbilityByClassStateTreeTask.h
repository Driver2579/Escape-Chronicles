// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "ActivateGameplayAbilityByClassStateTreeTask.generated.h"

class UGameplayAbility;
class AEscapeChroniclesCharacter;

USTRUCT()
struct FActivateGameplayAbilityByClassStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Character that activates the ability
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AEscapeChroniclesCharacter> OwnerCharacter;

	// Ability to activate
	UPROPERTY(EditAnywhere, Category="Parameter")
	TSubclassOf<UGameplayAbility> AbilityClass;
};

// Activates the gameplay ability of the specified class. If failed to activate, then the task will fail.
USTRUCT(Category="AI|Action", meta=(DisplayName="Activate Gameplay Ability by Class"))
struct FActivateGameplayAbilityByClassStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FActivateGameplayAbilityByClassStateTreeTask();

	using FInstanceDataType = FActivateGameplayAbilityByClassStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};