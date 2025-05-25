// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "RotateToStateTreeTask.generated.h"

USTRUCT()
struct FRotateToStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AActor> ActorToRotate;

	// Rotation to face in world space
	UPROPERTY(EditAnywhere, Category="Parameter")
	FRotator TargetRotation;

	// Speed at which the actor will rotate to face the target rotation
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=0.1))
	float InterpolationSpeed = 5;
};

// Rotates the given actor to face the specified rotation in world space with the specified speed
USTRUCT(meta=(DisplayName="Rotate To"))
struct FRotateToStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRotateToStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};