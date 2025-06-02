// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Common/Structs/ScheduleEventData.h"
#include "PushScheduleEventStateTreeTask.generated.h"

USTRUCT()
struct FPushScheduleEventStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Event that is going to be started (unless it's already in the stack)
	UPROPERTY(EditAnywhere, Category="Parameter")
	FScheduleEventData EventToPush;

	/**
	 * If true, the event will be loaded synchronously, otherwise it will be loaded asynchronously. You should use
	 * synchronous loading if you need to use the event instance immediately after starting the event. Otherwise, in
	 * most cases, you should prefer asynchronous loading.
	 */
	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bLoadEventInstanceSynchronously = false;
};

// Pushes an event to the stack of events unless it's already in the stack
USTRUCT(meta=(DisplayName="Push Schedule Event"))
struct FPushScheduleEventStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FPushScheduleEventStateTreeTask();

	using FInstanceDataType = FPushScheduleEventStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};