// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ConvertQuaternionToRotatorStateTreeTask.generated.h"

USTRUCT()
struct FConvertQuatToRotatorStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// FQuat to convert to FRotator
	UPROPERTY(EditAnywhere, Category="Input")
	FQuat Quaternion;

	// Converted rotation
	UPROPERTY(EditAnywhere, Category="Output")
	FRotator OutRotation;
};

// Converts the given FQuat to FRotator
USTRUCT(meta=(DisplayName="Convert Quaternion to Rotator"))
struct FConvertQuaternionToRotatorStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FConvertQuaternionToRotatorStateTreeTask();

	using FInstanceDataType = FConvertQuatToRotatorStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};