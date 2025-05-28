// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "SetGroundSpeedModeStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

enum class EGroundSpeedMode : uint8;

USTRUCT()
struct FSetGroundSpeedModeStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AEscapeChroniclesCharacter> OwnerCharacter;

	// The ground speed mode to set for the character
	UPROPERTY(EditAnywhere, Category="Parameter")
	EGroundSpeedMode GroundSpeedMode;

	// If true, the task will run forever until it's explicitly ended by a transition
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bRunTaskForever = false;

	/**
	 * If true, the ground speed mode will be reset to the default one when the task ends (unless it was overriden to
	 * another mode by something else).
	 */
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(EditCondition="bRunTaskForever"))
	bool bResetGroundSpeedModeOnTaskEnd = false;
};

// Sets the ground speed mode for the character and, if needed, resets it when the task ends
USTRUCT(Category="AI|Action", meta=(DisplayName="Set Ground Speed Mode"))
struct FSetGroundSpeedModeStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	FSetGroundSpeedModeStateTreeTask();

	using FInstanceDataType = FSetGroundSpeedModeStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual void ExitState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};