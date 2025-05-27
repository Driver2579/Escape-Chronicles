// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/RollCallScheduleEvent.h"

#include "EscapeChroniclesGameplayTags.h"
#include "Objects/ScheduleEvents/AlertScheduleEvent.h"

void URollCallScheduleEvent::NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent)
{
	Super::NotifyPlayerMissedEvent(PlayerThatMissedAnEvent);

#if DO_ENSURE
	ensureAlways(AlertEventData.IsValid());
	ensureAlways(AlertEventData.EventTag == EscapeChroniclesGameplayTags::ScheduleEvent_Alert);
#endif

	UScheduleEventManagerComponent* ScheduleEventManagerComponent = GetScheduleEventManagerComponent();

	// Start an alert if the player missed an event but only if it isn't already started
	if (!ScheduleEventManagerComponent->IsEventInStack(AlertEventData))
	{
		// Load an alert event class synchronously because we need its instance right now
		ScheduleEventManagerComponent->PushEvent(AlertEventData, true);
	}

	const FScheduleEventData& CurrentActiveEvent = ScheduleEventManagerComponent->GetCurrentActiveEventDataChecked();

	// Add the player that missed an event to the wanted players in alert event if it's the current active one
	if (CurrentActiveEvent.EventTag == AlertEventData.EventTag)
	{
#if DO_CHECK
		check(IsValid(CurrentActiveEvent.GetEventInstance()));
		check(CurrentActiveEvent.GetEventInstance()->IsA<UAlertScheduleEvent>());
#endif

		UAlertScheduleEvent* AlertEventInstance = CastChecked<UAlertScheduleEvent>(
			CurrentActiveEvent.GetEventInstance());

		// Mark the player that missed an event as wanted in the alert event
		AlertEventInstance->AddWantedPlayer(PlayerThatMissedAnEvent);
	}
}