// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/PushAlertEventStateTreeTask.h"

// We need this only for a check macro
#if DO_CHECK
#include "EscapeChroniclesGameplayTags.h"
#endif

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/ScheduleEventManagerComponent.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "Objects/ScheduleEvents/AlertScheduleEvent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

FPushAlertEventStateTreeTask::FPushAlertEventStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FPushAlertEventStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
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

#if DO_CHECK
	check(IsValid(ScheduleEventManagerComponent));
#endif

	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Make sure the provided event is a valid alert event
#if DO_CHECK
	check(InstanceData.AlertEventData.IsValid());
	check(InstanceData.AlertEventData.EventTag == EscapeChroniclesGameplayTags::ScheduleEvent_Alert);
	check(InstanceData.AlertEventData.EventClass->IsChildOf(UAlertScheduleEvent::StaticClass()));
#endif

	const bool bWantedCharactersAreNotEmpty = ensureAlwaysMsgf(!InstanceData.WantedCharacters.IsEmpty(),
		TEXT("An alert can't be started without any wanted characters!"));

	// Don't start an alert if there are no wanted characters or if the alert event is already in the stack
	if (!bWantedCharactersAreNotEmpty || ScheduleEventManagerComponent->IsEventInStack(InstanceData.AlertEventData))
	{
		return EStateTreeRunStatus::Failed;
	}

	TSet<FUniquePlayerID> WantedCharactersIDs;

	// Get IDs of all wanted characters
	for (const AEscapeChroniclesCharacter* WantedCharacter : InstanceData.WantedCharacters)
	{
		if (!ensureAlways(IsValid(WantedCharacter)))
		{
			continue;
		}

		const AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(
			WantedCharacter->GetPlayerState(), ECastCheckedType::NullAllowed);

		if (ensureAlways(IsValid(PlayerState)))
		{
			WantedCharactersIDs.Add(PlayerState->GetUniquePlayerID());
		}
	}

	// Check if we have any IDs of wanted characters to start the alert with
	const bool bWantedCharactersIDsAreNotEmpty = ensureAlwaysMsgf(!WantedCharactersIDs.IsEmpty(),
		TEXT("The provided array of wanted characters contains no valid characters with valid PlayerStates!"));

	if (!bWantedCharactersIDsAreNotEmpty)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Start an alert event and load it synchronously because we need its instance immediately
	ScheduleEventManagerComponent->PushEvent(InstanceData.AlertEventData, true);

	// Make sure the event was started successfully and has a valid instance
#if DO_CHECK
	check(ScheduleEventManagerComponent->GetCurrentActiveEventDataChecked() == InstanceData.AlertEventData);
	check(IsValid(ScheduleEventManagerComponent->GetCurrentActiveEventDataChecked().GetEventInstance()));
#endif

	// Get the instance of the alert event
	UAlertScheduleEvent* AlertEventInstance = CastChecked<UAlertScheduleEvent>(
		ScheduleEventManagerComponent->GetCurrentActiveEventDataChecked().GetEventInstance());

	// Set the wanted characters to the alert event instance
	AlertEventInstance->SetWantedPlayers(WantedCharactersIDs);

	return EStateTreeRunStatus::Succeeded;
}