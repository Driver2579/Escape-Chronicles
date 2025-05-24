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

    /**
     * If true, the task will return the "Running" status. If false, the task will return the "Succeeded" status.
     * Note: If the task fails, it will always return the "Failed" status regardless of this property.
     */
    UPROPERTY(EditAnywhere, Category="Parameter")
    bool bRunTaskForever = false;

    UPROPERTY(EditAnywhere, Category="Context")
    TObjectPtr<AEscapeChroniclesCharacter> OwnerCharacter;

    // The ground speed mode to set for the character
    UPROPERTY(EditAnywhere, Category="Parameter")
    EGroundSpeedMode GroundSpeedMode;

    /**
     * If true, the ground speed mode will be reset to the default one when the task ends (unless it was overriden to
     * another mode by something else).
     */
    UPROPERTY(EditAnywhere, Category="Parameter")
    bool bResetGroundSpeedModeOnTaskEnd = false;
};

USTRUCT(meta=(DisplayName="Set Movement Mode"))
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