// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/ScheduleEventManagerComponent.h"

#include "Engine/AssetManager.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "Objects/ScheduleEvents/ScheduleEvent.h"

UScheduleEventManagerComponent::UScheduleEventManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScheduleEventManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	if (ensureAlways(IsValid(GameState)))
	{
		GameState->OnCurrentDateTimeUpdated.AddUObject(this, &ThisClass::OnCurrentTimeUpdated);
	}
}

void UScheduleEventManagerComponent::OnCurrentTimeUpdated(const FGameplayDateTime& NewDateTime)
{
	// Try to find the event that starts at the given time
	const FScheduleEventData* NewScheduledEvent = ScheduledEvents.Find(NewDateTime.Time);

	// If the event is not found, try to find the event that is closest to the given time
	if (!NewScheduledEvent)
	{
		FGameplayTime MaximumScheduledTimeLessThanCurrentTime;

		for (const TPair<FGameplayTime, FScheduleEventData>& ScheduledEvent : ScheduledEvents)
		{
			/**
			 * If it isn't too late for the current iterated event to start and if it's closer to the current time than
			 * the previous iterated event, then remember this event as the closest one.
			 */
			if (ScheduledEvent.Key < NewDateTime.Time && ScheduledEvent.Key >= MaximumScheduledTimeLessThanCurrentTime)
			{
				MaximumScheduledTimeLessThanCurrentTime = ScheduledEvent.Key;
				NewScheduledEvent = &ScheduledEvent.Value;
			}
		}

		/**
		 * If we still don't have an event, then we shouldn't do anything. Just wait for the next time tick when we are
		 * going to recheck it again.
		 */
		if (!NewScheduledEvent)
		{
			return;
		}
	}

	// Make sure we don't restart the same event
	if (*NewScheduledEvent == GetCurrentScheduledEventData())
	{
		return;
	}

	// Simply add the event to the stack if it's the first event ever added
	if (EventsStack.IsEmpty())
	{
		EventsStack.Add(*NewScheduledEvent);
	}
	else
	{
		RemoveEventByIndex(0, EAllowShrinking::No);
		EventsStack.Insert(*NewScheduledEvent, 0);
	}

	/**
	 * Start a schedule event only if it's the only event in the stack. Otherwise, it will be started when it becomes
	 * the only one.
	 */
	if (EventsStack.Num() == 1)
	{
		CreateEventInstanceAndStartOrPauseIt(*NewScheduledEvent);
	}
}

void UScheduleEventManagerComponent::PushEvent(const FScheduleEventData& EventData)
{
#if DO_ENSURE
	ensureAlways(!EventsStack.Contains(EventData));
#endif

	// Pause the current event if it exists
	if (!EventsStack.IsEmpty())
	{
		UScheduleEvent* EventInstance = GetCurrentActiveEventData().GetEventInstance();

		/**
		 * The instance could still be loading. If it's valid already, then pause it now. Otherwise, it will be paused
		 * automatically when it's ready if this event still won't be active when this happens.
		 */
		if (IsValid(EventInstance))
		{
			EventInstance->PauseEvent();
		}
	}

	// Add the new event to the end stack. The last event is automatically considered as a current active one.
	EventsStack.Add(EventData);

	// Create the event instance and start or pause it when it's ready
	CreateEventInstanceAndStartOrPauseIt(EventData);
}

void UScheduleEventManagerComponent::CreateEventInstanceAndStartOrPauseIt(const FScheduleEventData& EventData)
{
#if DO_CHECK
	check(EventData.IsValid());
#endif

#if DO_ENSURE
	ensureAlways(!IsValid(EventData.GetEventInstance()) && !LoadEventInstancesHandles.Contains(EventData));
	ensureAlways(EventsStack.Contains(EventData));
#endif

	// Asynchronously load the event class
	const TSharedPtr<FStreamableHandle> LoadEventInstanceHandle =
		UAssetManager::GetStreamableManager().RequestAsyncLoad(EventData.EventClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnEventClassLoaded, EventData));

	LoadEventInstancesHandles.Add(EventData, LoadEventInstanceHandle);
}

// ReSharper disable once CppPassValueParameterByConstReference
void UScheduleEventManagerComponent::OnEventClassLoaded(const FScheduleEventData EventDataCopy)
{
#if DO_CHECK
	check(EventDataCopy.IsValid());
	check(EventDataCopy.EventClass.IsValid());
#endif

	// Find the reference to the event data by its copy
	const int32 EventIndex = EventsStack.Find(EventDataCopy);

#if DO_CHECK
	checkf(EventIndex != INDEX_NONE,
		TEXT("You must add an EventData to EventsStack before creating its instance!"));
#endif

	FScheduleEventData& EventData = EventsStack[EventIndex];

	// Create the new event of the given class
	UScheduleEvent* EventInstance = NewObject<UScheduleEvent>(this, EventData.EventClass.Get());

	// Set the references to each other as required
	EventData.SetEventInstance(EventInstance);
	EventInstance->SetEventData(EventData);

	// Always start the event when it's created
	EventInstance->StartEvent();

	// If the event is not the current active one, then pause it immediately
	if (EventData != GetCurrentActiveEventData())
	{
		EventInstance->PauseEvent();
	}
}

void UScheduleEventManagerComponent::RemoveEvent(const FGameplayTag& EventTag)
{
	// Find the event in the stack, and it and remove it. But ignore the first one because it's scheduled.
	for (int32 i = 1; i < EventsStack.Num(); ++i)
	{
		if (EventsStack[i].EventTag == EventTag)
		{
			RemoveEventByIndex(i);

			break;
		}
	}
}

void UScheduleEventManagerComponent::RemoveEventByIndex(const int32 Index, const EAllowShrinking AllowShrinking,
	const bool bStartOrResumeLastEvent)
{
#if DO_CHECK
	check(EventsStack.IsValidIndex(Index));
#endif

	// End the event before removing it
	EndEvent(EventsStack[Index]);

	// Try to find the loading handle for the event to clear it
	TSharedPtr<FStreamableHandle>* LoadEventInstanceHandle = LoadEventInstancesHandles.Find(EventsStack[Index]);

	if (LoadEventInstanceHandle)
	{
		// Unload the event's instance's class or unload it if it's still loading
		if (LoadEventInstanceHandle->IsValid())
		{
			LoadEventInstanceHandle->Get()->CancelHandle();
			LoadEventInstanceHandle->Reset();
		}

		// Remove the handle from the map if it is in the map
		LoadEventInstancesHandles.Remove(EventsStack[Index]);
	}

	// Once everything related to the event is cleared, remove it from the stack
	EventsStack.RemoveAt(Index, AllowShrinking);

	// Start or resume the new active event if we should and if it exists
	if (bStartOrResumeLastEvent && !EventsStack.IsEmpty())
	{
		/**
		 * The current active event is the last one in the stack, so it's changed automatically after we removed the
		 * previous one.
		 */
		const FScheduleEventData& CurrentActiveEventData = GetCurrentActiveEventData();

		UScheduleEvent* EventInstance = CurrentActiveEventData.GetEventInstance();

		// Check if the event instance is created already
		if (IsValid(EventInstance))
		{
			// Resume the event if it was paused before (which automatically means that it also was started before)
			if (EventInstance->IsPaused())
			{
				EventInstance->ResumeEvent();
			}
			// Otherwise, start the event if it was never started before
			else
			{
				EventInstance->StartEvent();
			}
		}
		/**
		 * Otherwise, if the event instance was not created yet, and it was not requested to be loaded, then request
		 * creating now.
		 */
		else if (!LoadEventInstancesHandles.Contains(CurrentActiveEventData))
		{
			CreateEventInstanceAndStartOrPauseIt(CurrentActiveEventData);
		}
	}
}

void UScheduleEventManagerComponent::EndEvent(FScheduleEventData& EventData)
{
	UScheduleEvent* EventInstance = EventData.GetEventInstance();

	if (IsValid(EventInstance))
	{
		EventInstance->EndEvent();
		EventData.ResetEventInstance();
	}
}

void UScheduleEventManagerComponent::OnPreLoadObject()
{
	/**
	 * Remember all events that are currently in the stack before loading the game to end the ones that won't be in the
	 * stack anymore after loading.
	 */
	EventsStackBeforeLoadingGame = EventsStack;
}

void UScheduleEventManagerComponent::OnPostLoadObject()
{
	// End all events that are not in the EventsStack after loading anymore
	for (FScheduleEventData& EventData : EventsStackBeforeLoadingGame)
	{
		if (!EventsStack.Contains(EventData))
		{
			EndEvent(EventData);
		}
	}

	// We don't need to keep it anymore
	EventsStackBeforeLoadingGame.Empty();

	// Create instances for events that were added to the stack by the loading
	for (FScheduleEventData& EventData : EventsStack)
	{
		if (!IsValid(EventData.GetEventInstance()) && !LoadEventInstancesHandles.Contains(EventData))
		{
			CreateEventInstanceAndStartOrPauseIt(EventData);
		}
	}
}