// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetActorLocationStateTreeTask.generated.h"

USTRUCT()
struct FGetActorLocationStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Actor to get the location of
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActor> Actor;

	UPROPERTY(EditAnywhere, Category="Output")
	FVector OutLocation;
};

// Gets the location of the given actor
USTRUCT(meta=(DisplayName="Get Actor Location"))
struct FGetActorLocationStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetActorLocationStateTreeTask();

	using FInstanceDataType = FGetActorLocationStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};