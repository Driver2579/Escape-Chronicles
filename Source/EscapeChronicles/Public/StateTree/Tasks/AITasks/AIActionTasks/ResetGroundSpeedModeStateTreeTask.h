// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "ResetGroundSpeedModeStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

enum class EGroundSpeedMode : uint8;

USTRUCT()
struct FResetGroundSpeedModeStateTreeTaskInstanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Context")
    TObjectPtr<AEscapeChroniclesCharacter> OwnerCharacter;

    // If true, the ground speed mode will be reset to the default one no matter what mode is currently set
    UPROPERTY(EditAnywhere, Category="Parameter")
    bool bForceResetGroundSpeedMode = false;

    // The ground speed mode to reset for the character. If a character has a different mode set, it will not be reset.
    UPROPERTY(EditAnywhere, Category="Parameter", meta=(EditCondition="!bForceResetGroundSpeedMode"))
    EGroundSpeedMode GroundSpeedModeOverrideToReset;
};

USTRUCT(meta=(DisplayName="Reset Movement Mode"))
struct FResetGroundSpeedModeStateTreeTask : public FStateTreeAIActionTaskBase
{
    GENERATED_BODY()

    FResetGroundSpeedModeStateTreeTask();

    using FInstanceDataType = FResetGroundSpeedModeStateTreeTaskInstanceData;

    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition) const override;
};