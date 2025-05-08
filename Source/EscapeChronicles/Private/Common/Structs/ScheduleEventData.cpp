// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/Structs/ScheduleEventData.h"

#include "Objects/ScheduleEvents/ScheduleEvent.h"

void FScheduleEventData::SetEventInstance(UScheduleEvent* InEventInstance)
{
#if DO_ENSURE
	ensureAlways(!EventInstance);
#endif

#if DO_CHECK
	check(::IsValid(InEventInstance));
#endif

	EventInstance = InEventInstance;
	OnEventInstanceCreated.Broadcast(EventInstance);
}

void FScheduleEventData::CallOrRegister_OnEventInstanceCreated(
	const FOnEventInstanceCreatedDelegate::FDelegate& Callback)
{
	if (EventInstance)
	{
		Callback.ExecuteIfBound(EventInstance);
	}
	else
	{
		OnEventInstanceCreated.Add(Callback);
	}
}