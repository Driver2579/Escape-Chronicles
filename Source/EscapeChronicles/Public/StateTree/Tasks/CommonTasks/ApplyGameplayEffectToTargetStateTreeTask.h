// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ApplyGameplayEffectToTargetStateTreeTask.generated.h"

class UGameplayEffect;

USTRUCT()
struct FApplyGameplayEffectStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Parameter")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	// Optional instigator that applies the gameplay effect to the target actor
	UPROPERTY(EditAnywhere, Category="Parameter")
	TObjectPtr<AActor> Instigator;

	// An actor that the gameplay effect will be applied to
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActor> Target;
};

/**
 * Applies a gameplay effect to a target actor. Optionally, takes an instigator actor. If an instigator is provided, it
 * will be the one that applies the gameplay effect. Otherwise, the target actor will apply the gameplay effect to
 * itself. The task fails if the gameplay effect fails to be applied.
 */
USTRUCT(meta=(DisplayName="Apply Gameplay Effect to Target"))
struct FApplyGameplayEffectToTargetStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FApplyGameplayEffectToTargetStateTreeTask();

	using FInstanceDataType = FApplyGameplayEffectStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};