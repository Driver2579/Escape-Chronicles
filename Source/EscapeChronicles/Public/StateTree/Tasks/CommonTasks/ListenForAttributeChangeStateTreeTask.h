// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "AttributeSet.h"
#include "ListenForAttributeChangeStateTreeTask.generated.h"

USTRUCT()
struct FListenForAttributeChangeStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Actor to listen for attribute changes on
	UPROPERTY(EditDefaultsOnly, Category="Context")
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

// A task that runs forever and listens for the selected attribute being changed on the selected actor
USTRUCT(meta=(DisplayName="Listen For Attribute Change"))
struct FListenForAttributeChangeStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FListenForAttributeChangeStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
		const IStateTreeBindingLookup& BindingLookup,
		EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};