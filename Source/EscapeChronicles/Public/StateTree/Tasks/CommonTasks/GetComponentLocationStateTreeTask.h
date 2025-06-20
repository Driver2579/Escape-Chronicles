// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetComponentLocationStateTreeTask.generated.h"

USTRUCT()
struct FGetComponentLocationStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// A component to get the location of
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<USceneComponent> Component;

	UPROPERTY(VisibleAnywhere, Transient, Category="Output")
	FVector OutLocation;
};

// A property function that receives a scene component and returns the world location from it
USTRUCT(DisplayName="Get Component Location")
struct FGetComponentLocationStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetComponentLocationStateTreeTask();

	using FInstanceDataType = FGetComponentLocationStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};