// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvent.h"

void UScheduleEvent::StartEvent()
{
#if DO_ENSURE
	ensureAlways(EventData.IsValid());
#endif

	if (bActive)
	{
		return;
	}

	bActive = true;
	bTickEnabled = true;

	OnEventStarted();
}

void UScheduleEvent::EndEvent()
{
	if (!bActive)
	{
		return;
	}

	bTickEnabled = false;
	bActive = false;

	OnEventEnded();
}

void UScheduleEvent::PauseEvent()
{
	if (!bActive || bPaused)
	{
		return;
	}

	bPaused = true;
	bTickEnabled = false;

	OnEventPaused();
}

void UScheduleEvent::ResumeEvent()
{
	if (!bActive || !bPaused)
	{
		return;
	}

	bTickEnabled = true;
	bPaused = false;

	OnEventResumed();
}