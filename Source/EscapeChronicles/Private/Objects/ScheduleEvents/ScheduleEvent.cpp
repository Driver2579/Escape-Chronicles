// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/ScheduleEvent.h"

void UScheduleEvent::StartEvent(const bool bStartPaused)
{
#if DO_ENSURE
	ensureAlways(EventData.IsValid());
#endif

	// Don't start the event if it's already active
	if (bActive)
	{
		return;
	}

	// Mark that the event is active now
	bActive = true;

	// Enable the tick (this still will not tick if bCanEverTick is false)
	bTickEnabled = true;

	// Notify that the event has started
	OnEventStarted(bStartPaused);

	// Pause an event if it was requested
	if (bStartPaused)
	{
		PauseEvent();
	}
}

void UScheduleEvent::EndEvent()
{
	// Don't end the event if it wasn't started
	if (!bActive)
	{
		return;
	}

	// Disable the tick
	bTickEnabled = false;

	// Mark that the event is not active anymore
	bActive = false;

	OnEventEnded();

	// Mark that the event isn't paused anymore (because it isn't even active anymore)
	bPaused = false;
}

void UScheduleEvent::PauseEvent()
{
	// Don't pause the event if it wasn't started or if it's already paused
	if (!bActive || bPaused)
	{
		return;
	}

	// Mark that the event is now paused
	bPaused = true;

	// Disable the tick
	bTickEnabled = false;

	OnEventPaused();
}

void UScheduleEvent::ResumeEvent()
{
	// Don't resume the event if it was ended or if it isn't paused
	if (!bActive || !bPaused)
	{
		return;
	}

	// Enable the tick back (this still will not tick if bCanEverTick is false)
	bTickEnabled = true;

	// Mark that the event isn't paused anymore
	bPaused = false;

	OnEventResumed();
}