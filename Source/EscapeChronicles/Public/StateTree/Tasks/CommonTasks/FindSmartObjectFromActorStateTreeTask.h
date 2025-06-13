// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "SmartObjectRequestTypes.h"
#include "FindSmartObjectFromActorStateTreeTask.generated.h"

USTRUCT()
struct FGetSmartObjectFromActorByClassStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Actor to get the smart object from
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AActor> Actor;

	// Filter to use when searching for smart objects
	UPROPERTY(EditAnywhere, Category="Parameter")
	FSmartObjectRequestFilter SmartObjectRequestFilter;

	// An actor that is going to use the smart object
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AActor> UserActor;

	// A found smart object
	UPROPERTY(VisibleAnywhere, Category="Output")
	FSmartObjectRequestResult OutSmartObjectRequestResult;
};

// Finds the first or unclaimed smart object in the specified actor
USTRUCT(DisplayName="Find Smart Object From Actor", Category="Smart Objects")
struct FFindSmartObjectFromActorStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FFindSmartObjectFromActorStateTreeTask();

	using FInstanceDataType = FGetSmartObjectFromActorByClassStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};