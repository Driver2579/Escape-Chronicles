// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Common/Structs/ScheduleEventData.h"
#include "PushAlertEventStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FPushAlertEventStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Data for the alert event that is going to be started (unless it's already in the stack)
	UPROPERTY(EditAnywhere, Category="Parameter")
	FScheduleEventData AlertEventData;

	// Characters that are wanted during the alert event
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> WantedCharacters;
};

// Pushes an event to the stack of events unless it's already in the stack
USTRUCT(meta=(DisplayName="Push Alert Event"))
struct FPushAlertEventStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FPushAlertEventStateTreeTask();

	using FInstanceDataType = FPushAlertEventStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};