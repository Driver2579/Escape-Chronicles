// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/ScheduleEventManagerComponent.h"

#include "Engine/AssetManager.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "Objects/ScheduleEvent.h"

// TODO: Add comments

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
		GameState->OnCurrentTimeUpdated.AddUObject(this, &ThisClass::OnCurrentTimeUpdated);
	}
}

void UScheduleEventManagerComponent::OnCurrentTimeUpdated(const FGameplayTime& GameplayTime)
{
	const FScheduleEventData* ScheduleEvent = ScheduleEvents.Find(GameplayTime);

	if (!ScheduleEvent)
	{
		return;
	}

	if (EventsStack.IsEmpty())
	{
		EventsStack.Add(*ScheduleEvent);
	}
	else
	{
		RemoveEventByIndex(0, EAllowShrinking::No);
		EventsStack.Insert(*ScheduleEvent, 0);
	}

	/**
	 * Start a schedule event only if it's the only event in the stack. Otherwise, it will be started when it becomes
	 * the only one.
	 */
	if (EventsStack.Num() == 1)
	{
		CreateEventInstanceAndStartIt(*ScheduleEvent);
	}
}

void UScheduleEventManagerComponent::PushEvent(const FScheduleEventData& EventData)
{
#if DO_ENSURE
	ensureAlways(!EventsStack.Contains(EventData));
#endif

	if (!EventsStack.IsEmpty())
	{
		const FScheduleEventData& CurrentEventData = GetCurrentActiveEventData();

		if (CurrentEventData.EventInstance)
		{
			CurrentEventData.EventInstance->PauseEvent();
		}
	}

	EventsStack.Add(EventData);
	CreateEventInstanceAndStartIt(EventData);
}

void UScheduleEventManagerComponent::RemoveEvent(const FGameplayTag& EventTag)
{
	// Find the event in the stack, and it and remove it
	for (int32 i = 0; i < EventsStack.Num(); ++i)
	{
		if (EventsStack[i].EventTag == EventTag)
		{
			RemoveEventByIndex(i);

			break;
		}
	}
}

void UScheduleEventManagerComponent::CreateEventInstanceAndStartIt(const FScheduleEventData& EventData)
{
#if DO_CHECK
	check(EventData.IsValid());
#endif

#if DO_ENSURE
	ensureAlways(!EventData.EventInstance);
	ensureAlways(EventsStack.Contains(EventData));
#endif

	UAssetManager::GetStreamableManager().RequestAsyncLoad(EventData.EventClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnEventClassLoaded, EventData));
}

// ReSharper disable once CppPassValueParameterByConstReference
void UScheduleEventManagerComponent::OnEventClassLoaded(const FScheduleEventData EventDataCopy)
{
#if DO_CHECK
	check(EventDataCopy.IsValid());
	check(EventDataCopy.EventClass.IsValid());
#endif

	const int32 EventIndex = EventsStack.Find(EventDataCopy);

#if DO_CHECK
	checkf(EventIndex != INDEX_NONE,
		TEXT("You must add an EventData to EventsStack before creating its instance!"));
#endif

	FScheduleEventData& EventData = EventsStack[EventIndex];

	EventData.EventInstance = NewObject<UScheduleEvent>(this, EventData.EventClass.Get());
	EventData.EventInstance->SetEventData(EventData);

	EventData.EventInstance->StartEvent();

	if (EventData != GetCurrentActiveEventData())
	{
		EventData.EventInstance->PauseEvent();
	}
}

void UScheduleEventManagerComponent::RemoveEventByIndex(const int32 Index, const EAllowShrinking AllowShrinking,
	const bool bStartOrUnpauseLastEvent)
{
#if DO_CHECK
	check(EventsStack.IsValidIndex(Index));
#endif

	EndEvent(EventsStack[Index]);
	EventsStack.RemoveAt(Index, AllowShrinking);

	if (bStartOrUnpauseLastEvent && !EventsStack.IsEmpty())
	{
		const FScheduleEventData& CurrentActiveEventData = GetCurrentActiveEventData();

		if (CurrentActiveEventData.EventInstance)
		{
			CurrentActiveEventData.EventInstance->ResumeEvent();
		}
		else
		{
			CreateEventInstanceAndStartIt(CurrentActiveEventData);
		}
	}
}

void UScheduleEventManagerComponent::EndEvent(FScheduleEventData& EventData)
{
	if (EventData.EventInstance)
	{
		EventData.EventInstance->EndEvent();
		EventData.EventInstance = nullptr;
	}
}

void UScheduleEventManagerComponent::OnPostLoadObject()
{
	for (FScheduleEventData& EventData : EventsStack)
	{
		if (!EventData.EventInstance)
		{
			CreateEventInstanceAndStartIt(EventData);
		}
	}
}