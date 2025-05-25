// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/AIActionTasks/UseSmartObjectStateTreeTask.h"

#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "AI/AITask_UseGameplayBehaviorSmartObject.h"

EStateTreeRunStatus FUseSmartObjectStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const UWorld* World = Context.GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return EStateTreeRunStatus::Failed;
	}

	USmartObjectSubsystem* SmartObjectSubsystem = World->GetSubsystem<USmartObjectSubsystem>();

	if (!ensureAlways(IsValid(SmartObjectSubsystem)))
	{
		return EStateTreeRunStatus::Failed;
	}

	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!ensureAlways(InstanceData.SmartObjectSlotHandle.IsValid()))
	{
		return EStateTreeRunStatus::Failed;
	}

	const FSmartObjectClaimHandle SmartObjectClaimHandle = SmartObjectSubsystem->MarkSlotAsClaimed(
		InstanceData.SmartObjectSlotHandle, InstanceData.SmartObjectClaimPriority,
		FConstStructView::Make(FSmartObjectActorUserData(InstanceData.UserActor)));

	if (!ensureAlwaysMsgf(SmartObjectClaimHandle.IsValid(), TEXT("Failed to claim the smart object slot!")))
	{
		return EStateTreeRunStatus::Failed;
	}

	UAITask_UseGameplayBehaviorSmartObject* UseSmartObjectInstance =
		UAITask_UseGameplayBehaviorSmartObject::UseSmartObjectWithGameplayBehavior(InstanceData.AIController,
			SmartObjectClaimHandle, true, InstanceData.SmartObjectClaimPriority);

	if (!ensureAlwaysMsgf(IsValid(UseSmartObjectInstance), TEXT("Failed to use the smart object slot!")))
	{
		return EStateTreeRunStatus::Failed;
	}

	/**
	 * TODO:
	 * Это неправильно. Мы не можем подписаться на делегаты UseSmartObjectInstance. Это способ для BP. Надо найти как
	 * через C++, изучая сорс код UAITask_UseGameplayBehaviorSmartObject.
	 */

	return EStateTreeRunStatus::Succeeded;
}