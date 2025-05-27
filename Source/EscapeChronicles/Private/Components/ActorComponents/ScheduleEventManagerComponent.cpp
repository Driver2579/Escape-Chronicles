// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/ScheduleEventManagerComponent.h"

#include "Engine/AssetManager.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "Objects/ScheduleEvents/AlertScheduleEvent.h"
#include "Objects/ScheduleEvents/ScheduleEvent.h"
#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/BedtimeScheduleEvent.h"
#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/ScheduleEventWithPresenceMark.h"
#include "Subsystems/SaveGameSubsystem.h"

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
		OnCurrentGameDateTimeUpdated(FGameplayDateTime(), GameState->GetCurrentGameDateTime());

		GameState->OnCurrentGameDateTimeUpdated.AddUObject(this, &ThisClass::OnCurrentGameDateTimeUpdated);
	}
}

void UScheduleEventManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove and end all events when the component is destroyed but don't remove their save data
	for (int32 i = EventsStack.Num() - 1; i >= 0; --i)
	{
		RemoveEventByIndex(i, EAllowShrinking::No, EScheduleEventEndReason::EndPlay, false,
			false);
	}

	EventsStack.Shrink();

	Super::EndPlay(EndPlayReason);
}

void UScheduleEventManagerComponent::OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime,
	const FGameplayDateTime& NewDateTime)
{
	// Don't do anything if there are no scheduled events
	if (ScheduledEvents.IsEmpty())
	{
		return;
	}

	const USaveGameSubsystem* SaveGameSubsystem = GetWorld()->GetSubsystem<USaveGameSubsystem>();

	/**
	 * Don't do anything if the game is currently loading from save (loading will handle switching events automatically
	 * and we don't want to conflict with it).
	 */
	if (ensureAlways(IsValid(SaveGameSubsystem)) && SaveGameSubsystem->IsGameLoadingInProgress())
	{
		return;
	}

	// Try to find the event that starts at the given time
	const FScheduleEventData* NewScheduledEvent = ScheduledEvents.Find(NewDateTime.Time);

	// If the event is not found, find the event that is closest to the given time
	if (!NewScheduledEvent)
	{
		FGameplayTime MaximumScheduledTimeLessThanCurrentTime;
		FGameplayTime MaximumScheduledTime;

		const FScheduleEventData* LatestScheduledEventInADay = nullptr;

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

			/**
			 * We could fail to find an event if it should've started yesterday (for example, if should start at 23:00
			 * but now is 1:00). So we also need to find an event that is closest to the end of the day (basically, an
			 * event from the previous day).
			 */
			if (ScheduledEvent.Key >= MaximumScheduledTime)
			{
				MaximumScheduledTime = ScheduledEvent.Key;
				LatestScheduledEventInADay = &ScheduledEvent.Value;
			}
		}

		// If we didn't find the closest event in the current day, then use the last event of the previous day
		if (!NewScheduledEvent)
		{
			NewScheduledEvent = LatestScheduledEventInADay;
		}
	}

#if DO_CHECK
	// We should've found an event by now
	check(NewScheduledEvent);
#endif

	// Make sure we don't restart the same event
	if (!EventsStack.IsEmpty() && *NewScheduledEvent == GetCurrentScheduledEventDataChecked())
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

const FScheduleEventData* UScheduleEventManagerComponent::FindEventByTag(const FGameplayTag& EventTag) const
{
#if DO_ENSURE
	ensureAlways(EventTag.IsValid());
#endif

	for (const FScheduleEventData& EventData : EventsStack)
	{
		if (EventData.EventTag == EventTag)
		{
			return &EventData;
		}
	}

	return nullptr;
}

void UScheduleEventManagerComponent::PushEvent(const FScheduleEventData& EventData,
	const bool bLoadEventClassSynchronously)
{
#if DO_ENSURE
	ensureAlways(!EventsStack.Contains(EventData));
#endif

	// Pause the current event if it exists
	if (!EventsStack.IsEmpty())
	{
		UScheduleEvent* EventInstance = GetCurrentActiveEventDataChecked().GetEventInstance();

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
	CreateEventInstanceAndStartOrPauseIt(EventData, bLoadEventClassSynchronously);
}

void UScheduleEventManagerComponent::CreateEventInstanceAndStartOrPauseIt(const FScheduleEventData& EventData,
	const bool bLoadEventClassSynchronously)
{
#if DO_CHECK
	check(EventData.IsValid());
#endif

#if DO_ENSURE
	ensureAlways(!IsValid(EventData.GetEventInstance()) && !LoadEventInstancesHandles.Contains(EventData));
	ensureAlways(EventsStack.Contains(EventData));
#endif

	TSharedPtr<FStreamableHandle> LoadEventInstanceHandle;

	// Load the event class synchronously if required and call the callback immediately
	if (bLoadEventClassSynchronously)
	{
		LoadEventInstanceHandle = UAssetManager::GetStreamableManager().RequestSyncLoad(
			EventData.EventClass.ToSoftObjectPath());

		OnEventClassLoaded(EventData);
	}
	// Otherwise, load it asynchronously and bind the callback to be called when it's ready
	else
	{
		LoadEventInstanceHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			EventData.EventClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnEventClassLoaded, EventData));
	}

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

	const bool bFoundEventIndex = ensureAlwaysMsgf(EventIndex != INDEX_NONE,
		TEXT("You must add an EventData to EventsStack before creating its instance!"));

	if (!bFoundEventIndex)
	{
		return;
	}

	FScheduleEventData& EventData = EventsStack[EventIndex];

	// Create the new event of the given class
	UScheduleEvent* EventInstance = NewObject<UScheduleEvent>(this, EventData.EventClass.Get());

	// Set the references to each other as required
	EventData.SetEventInstance(EventInstance);
	EventInstance->SetEventData(EventData);

	// Always start the event when it's created, but start it paused if the event is not the current active one
	if (EventData == GetCurrentActiveEventDataChecked())
	{
		EventInstance->StartEvent(false);
	}
	else
	{
		EventInstance->StartEvent(true);
	}

	// TODO: The next logic is bad. We should add support for saving custom objects into byte data.

	// Try to find the save data for the event if it's an event that can be saved and apply it if it was found
	if (EventInstance->IsA<UScheduleEventWithPresenceMark>())
	{
		UScheduleEventWithPresenceMark* ScheduleEventWithPresenceMark = CastChecked<UScheduleEventWithPresenceMark>(
			EventInstance);

		const FScheduleEventWithPresenceMarkSaveData* ScheduleEventWithPresenceMarkSaveData =
			SavedEventsWithPresenceMark.Find(EventData.EventTag);

		if (ScheduleEventWithPresenceMarkSaveData)
		{
			ScheduleEventWithPresenceMark->LoadScheduleEventWithPresenceMarkFromSaveData(
				*ScheduleEventWithPresenceMarkSaveData);
		}

		// UBedtimeScheduleEvent is a child of UScheduleEventWithPresenceMark
		if (EventInstance->IsA<UBedtimeScheduleEvent>())
		{
			UBedtimeScheduleEvent* BedtimeScheduleEvent = CastChecked<UBedtimeScheduleEvent>(EventInstance);

			const FBedtimeScheduleEventSaveData* BedtimeSaveData = SavedBedtimeScheduleEvents.Find(EventData.EventTag);

			if (BedtimeSaveData)
			{
				BedtimeScheduleEvent->LoadBedtimeScheduleEventFromSaveData(*BedtimeSaveData);
			}
		}
	}
	else if (EventInstance->IsA<UAlertScheduleEvent>())
	{
		UAlertScheduleEvent* AlertScheduleEvent = CastChecked<UAlertScheduleEvent>(EventInstance);

		const FAlertScheduleEventSaveData* AlertSaveData = SavedAlertScheduleEvents.Find(EventData.EventTag);

		if (AlertSaveData)
		{
			AlertScheduleEvent->LoadAlertScheduleEventFromSaveData(*AlertSaveData);
		}
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
	const EScheduleEventEndReason EndReason, const bool bRemoveSaveData, const bool bStartOrResumeLastEvent)
{
#if DO_CHECK
	check(EventsStack.IsValidIndex(Index));
#endif

	// End and unload the event before removing it
	EndEvent(EventsStack[Index], EndReason, bRemoveSaveData);
	UnloadOrCancelLoadingEventInstance(EventsStack[Index]);

	// Once everything related to the event is cleared, remove it from the stack
	EventsStack.RemoveAt(Index, AllowShrinking);

	// Start or resume the new active event if we should and if it exists
	if (bStartOrResumeLastEvent && !EventsStack.IsEmpty())
	{
		/**
		 * The current active event is the last one in the stack, so it's changed automatically after we removed the
		 * previous one.
		 */
		const FScheduleEventData& CurrentActiveEventData = GetCurrentActiveEventDataChecked();

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

void UScheduleEventManagerComponent::EndEvent(FScheduleEventData& EventData, const EScheduleEventEndReason EndReason,
	const bool bRemoveSaveData)
{
	UScheduleEvent* EventInstance = EventData.GetEventInstance();

	if (!IsValid(EventInstance))
	{
		return;
	}

	// TODO: The next logic is bad. We should add support for saving custom objects into byte data.

	// Remove the save data for the event if required and if it exists
	if (bRemoveSaveData)
	{
		if (EventInstance->IsA<UScheduleEventWithPresenceMark>())
		{
			SavedEventsWithPresenceMark.Remove(EventData.EventTag);

			// UBedtimeScheduleEvent is a child of UScheduleEventWithPresenceMark
			if (EventInstance->IsA<UBedtimeScheduleEvent>())
			{
				SavedBedtimeScheduleEvents.Remove(EventData.EventTag);
			}
		}
		else if (EventInstance->IsA<UAlertScheduleEvent>()) 
		{
			SavedAlertScheduleEvents.Remove(EventData.EventTag);
		}
	}

	EventInstance->EndEvent(EndReason);
	EventData.ResetEventInstance();
}

void UScheduleEventManagerComponent::UnloadOrCancelLoadingEventInstance(const FScheduleEventData& EventData)
{
	// Try to find the loading handle for the event to clear it
	TSharedPtr<FStreamableHandle>* LoadEventInstanceHandle = LoadEventInstancesHandles.Find(EventData);

	// Return if there is nothing to clear
	if (!LoadEventInstanceHandle)
	{
		return;
	}

	if (LoadEventInstanceHandle->IsValid())
	{
		// Unload the event's instance's class or cancel its loading it if it's still loading
		LoadEventInstanceHandle->Get()->CancelHandle();
		LoadEventInstanceHandle->Reset();
	}

	// Remove the handle from the map if it is in the map
	LoadEventInstancesHandles.Remove(EventData);
}

void UScheduleEventManagerComponent::OnPreSaveObject()
{
	// TODO: The next logic is bad. We should add support for saving custom objects into byte data.

	// Save all events that can be saved
	for (const FScheduleEventData& EventData : EventsStack)
	{
		UScheduleEvent* EventInstance = EventData.GetEventInstance();

		if (!IsValid(EventInstance))
		{
			continue;
		}

		if (EventInstance->IsA<UScheduleEventWithPresenceMark>())
		{
			const UScheduleEventWithPresenceMark* ScheduleEventWithPresenceMark =
				CastChecked<UScheduleEventWithPresenceMark>(EventInstance);

			SavedEventsWithPresenceMark.Add(EventData.EventTag,
				ScheduleEventWithPresenceMark->GetScheduleEventWithPresenceMarkSaveData());

			// UBedtimeScheduleEvent is a child of UScheduleEventWithPresenceMark
			if (EventInstance->IsA<UBedtimeScheduleEvent>())
			{
				const UBedtimeScheduleEvent* BedtimeScheduleEvent = CastChecked<UBedtimeScheduleEvent>(EventInstance);

				SavedBedtimeScheduleEvents.Add(EventData.EventTag,
					BedtimeScheduleEvent->GetBedtimeScheduleEventSaveData());
			}
		}
		else if (EventInstance->IsA<UAlertScheduleEvent>())
		{
			const UAlertScheduleEvent* AlertScheduleEvent = CastChecked<UAlertScheduleEvent>(EventInstance);

			SavedAlertScheduleEvents.Add(EventData.EventTag,
				AlertScheduleEvent->GetAlertScheduleEventSaveData());
		}
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
	// End and unload all events that are not in the EventsStack after loading anymore
	for (FScheduleEventData& EventData : EventsStackBeforeLoadingGame)
	{
		if (!EventsStack.Contains(EventData))
		{
			EndEvent(EventData, EScheduleEventEndReason::Loading, true);
			UnloadOrCancelLoadingEventInstance(EventData);
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