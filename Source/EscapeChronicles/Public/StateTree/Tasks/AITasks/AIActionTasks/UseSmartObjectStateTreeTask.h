// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "SmartObjectTypes.h"
#include "UseSmartObjectStateTreeTask.generated.h"

class AAIController;

USTRUCT()
struct FUseSmartObjectStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// An actor that wants to use the smart object
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AActor> UserActor;

	// AI controller of the user actor
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AAIController> AIController;

	// Smart object slot to use
	UPROPERTY(EditAnywhere, Category="Input")
	FSmartObjectSlotHandle SmartObjectSlotHandle;

	// A priority to claim the smart object with
	UPROPERTY(EditAnywhere, Category="Parameter")
	ESmartObjectClaimPriority SmartObjectClaimPriority = ESmartObjectClaimPriority::Normal;
};

/**
 * Uses a specified smart object and claims it. This task will be finished once the smart object finishes its work (but
 * you can interrupt it manually, of course). Once the task is finished, the smart object will be released
 * automatically.
 */
USTRUCT(Category="AI|Action|Smart Objects", meta=(DisplayName="Use Smart Object"))
struct FUseSmartObjectStateTreeTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FUseSmartObjectStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};