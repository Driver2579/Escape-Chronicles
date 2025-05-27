// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "EmptyStateTreeTask.generated.h"

USTRUCT()
struct FEmptyStateTreeTaskInstanceData
{
	GENERATED_BODY()
};

/**
 * An empty task that does nothing and simply returns Succeeded status. Can be used if you want a state that has no
 * tasks (such states run forever by default).
 */
USTRUCT(meta=(DisplayName="Empty Task"))
struct FEmptyStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEmptyStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override
	{
		return EStateTreeRunStatus::Succeeded;
	}
};