// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/PushScheduleEventStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Components/ActorComponents/ScheduleEventManagerComponent.h"
#include "GameModes/EscapeChroniclesGameMode.h"

FPushScheduleEventStateTreeTask::FPushScheduleEventStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FPushScheduleEventStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const UWorld* World = Context.GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return EStateTreeRunStatus::Failed;
	}

	const AEscapeChroniclesGameMode* GameMode = World->GetAuthGameMode<AEscapeChroniclesGameMode>();

	if (!ensureAlways(IsValid(GameMode)))
	{
		return EStateTreeRunStatus::Failed;
	}

	UScheduleEventManagerComponent* ScheduleEventManagerComponent = GameMode->GetScheduleEventManagerComponent();

#if DO_CHEK
	check(IsValid(ScheduleEventManagerComponent));
#endif

	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_ENSURE
	ensureAlways(InstanceData.EventToPush.IsValid());
#endif

	// Push the event and load it either synchronously or asynchronously
	ScheduleEventManagerComponent->PushEvent(InstanceData.EventToPush, InstanceData.bLoadEventInstanceSynchronously);

	return EStateTreeRunStatus::Succeeded;
}