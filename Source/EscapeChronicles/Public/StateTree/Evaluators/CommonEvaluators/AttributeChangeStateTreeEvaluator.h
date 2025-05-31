// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "AttributeSet.h"
#include "AttributeChangeStateTreeEvaluator.generated.h"

USTRUCT()
struct FAttributeChangeStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	// Actor to listen for attribute changes on
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<AActor> Actor;

	// Attribute to listen for changes on
	UPROPERTY(EditDefaultsOnly, Category="Parameter")
	FGameplayAttribute Attribute;

	// Current base value of the attribute
	UPROPERTY(EditAnywhere, Category="Output")
	float BaseValue;

	// Current value of the attribute
	UPROPERTY(EditAnywhere, Category="Output")
	float CurrentValue;

	// A delegate that is called when the base value of the attribute changes
	UPROPERTY(EditAnywhere)
	FStateTreeDelegateDispatcher OnAttributeBaseValueChangedDispatcher;

	// A delegate that is called when the current value of the attribute changes
	UPROPERTY(EditAnywhere)
	FStateTreeDelegateDispatcher OnAttributeCurrentValueChangedDispatcher;

	// Ability System Component of the given actor
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};

// An evaluator that listens for the selected attribute being changed on the selected actor
USTRUCT(meta=(DisplayName="Attribute Change"))
struct FAttributeChangeStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FAttributeChangeStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};