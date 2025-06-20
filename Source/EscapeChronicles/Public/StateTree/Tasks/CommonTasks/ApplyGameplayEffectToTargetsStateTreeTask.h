// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ApplyGameplayEffectToTargetsStateTreeTask.generated.h"

class UGameplayEffect;

USTRUCT()
struct FApplyGameplayEffectToTargetsStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Parameter")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	// Optional instigator that applies the gameplay effect to the target actor
	UPROPERTY(EditAnywhere, Category="Parameter")
	TObjectPtr<AActor> Instigator;

	// Actors that the gameplay effect will be applied to
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> Targets;

	// Actors that match this tag query will be ignored from applying the gameplay effect
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTagQuery IgnoredActorsTagQuery;
};

/**
 * Applies a gameplay effect to given target actors. Optionally, takes an instigator actor. If an instigator is
 * provided, it will be the one that applies the gameplay effect. Otherwise, target actors will apply the gameplay
 * effect to themselves. The task fails if the gameplay effect wasn't applied to at least one actor.
 */
USTRUCT(meta=(DisplayName="Apply Gameplay Effect to Targets"))
struct FApplyGameplayEffectToTargetsStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FApplyGameplayEffectToTargetsStateTreeTask();

	using FInstanceDataType = FApplyGameplayEffectToTargetsStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};