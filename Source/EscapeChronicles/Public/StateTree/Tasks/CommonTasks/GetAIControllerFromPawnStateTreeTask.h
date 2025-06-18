// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetAIControllerFromPawnStateTreeTask.generated.h"

class AAIController;

USTRUCT()
struct FGetAIControllerFromPawnStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Pawn to get AI Controller from
	UPROPERTY(EditAnywhere, Category="In")
	TObjectPtr<APawn> Pawn;

	UPROPERTY(EditAnywhere, Category="Output")
	TObjectPtr<AAIController> OutAIController;
};

// Gets the AI Controller from the given pawn
USTRUCT(meta=(DisplayName="Get AI Controller from Pawn"))
struct FGetAIControllerFromPawnStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetAIControllerFromPawnStateTreeTask();

	using FInstanceDataType = FGetAIControllerFromPawnStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};