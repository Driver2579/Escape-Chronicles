// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Structs/ScheduleEventData.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/GameplayDateTime.h"
#include "ScheduleEventManagerComponent.generated.h"

struct FStreamableHandle;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEventChangedDelegate, const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData);

/**
 * This component handles switching events for the game. There are two types of events in this component: scheduled and
 * non-scheduled events. Scheduled events are events that are switched automatically when the game time changes.
 * Non-scheduled events could override the scheduled events if you manually push them on top of the stack. There is only
 * one event that is active at a time - the event that is on the top of the stack. Other events are paused. The last
 * event you pushed is always on top of the stack, so this would be the one that is active, until you manually remove
 * it, which will end it and unpause the event that is now on top of the stack (either the event you pushed before or
 * the scheduled event if there are no pushed events left).
 *
 * @remark This class is designed to be used on authority only. Nothing is replicated here. It is recommended to create
 * this component in the GameMode or something similar, use it only on the server, and replicate the delegates via the
 * GameState or something similar.
 */
UCLASS(ClassGroup=(Custom))
class ESCAPECHRONICLES_API UScheduleEventManagerComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UScheduleEventManagerComponent();

	/**
	 * @return An event that is set by the schedule is changed. Isn't necessarily active, could be paused due to the
	 * override by another event.
	 */
	const FScheduleEventData& GetCurrentScheduledEventData() const
	{
#if DO_CHECK
		check(!EventsStack.IsEmpty());
#endif

		return EventsStack[0];
	}

	/**
	 * @return An event that is currently active.
	 */
	const FScheduleEventData& GetCurrentActiveEventData() const
	{
#if DO_CHECK
		check(!EventsStack.IsEmpty());
#endif

		return EventsStack.Last();
	}

	/**
	 * Pushes a new event on top of the current one. The current active event will be paused and the new one will be
	 * started.
	 */
	void PushEvent(const FScheduleEventData& EventData);

	/**
	 * Removes the event with the given tag from the stack and ends it unless it's a scheduled event. The last event
	 * before the removed one in the stack will be paused if it was paused before or started if it was never started
	 * before. This event will be considered as a current active event.
	 */
	void RemoveEvent(const FGameplayTag& EventTag);

	/**
	 * Called when an event that is set by the schedule is changed. The new event isn't necessarily active, it could be
	 * started and instantly paused due to the override by another event.
	 */
	FOnEventChangedDelegate OnCurrentScheduledEventChanged;

	// Called when an event that is currently active is changed
	FOnEventChangedDelegate OnCurrentActiveEventChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Handles switching between scheduled events
	virtual void OnCurrentTimeUpdated(const FGameplayDateTime& NewDateTime);

	virtual void OnPreLoadObject() override;
	virtual void OnPostLoadObject() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTime, FScheduleEventData> ScheduledEvents;

	void CreateEventInstanceAndStartOrPauseIt(const FScheduleEventData& EventData);
	void OnEventClassLoaded(FScheduleEventData EventDataCopy);

	/**
	 * Removes the event at the given index from the EventsStack and ends it.
	 * @param Index Index of the event to remove. Must be valid!
	 * @param AllowShrinking Whether to allow shrinking the EventsStackArray. If not, then you have to shrink it
	 * manually.
	 * @param bStartOrResumeLastEvent If true, the last event before the removed one in the EventsStack will be paused
	 * if it was paused before or started if it was never started before. This event will be considered as a current
	 * active event.
	 */
	void RemoveEventByIndex(const int32 Index, const EAllowShrinking AllowShrinking = EAllowShrinking::Yes,
		const bool bStartOrResumeLastEvent = true);

	// Ends the event and removes it from the EventData
	static void EndEvent(FScheduleEventData& EventData);

	/**
	 * The stack of events that are currently set, but only one event is active at a time. These events should always be
	 * valid. These events are sorted by the priority in which they should be activated. The first one is always a
	 * scheduled event, and the last one is always the current active event, but still prefer getters to get them
	 * instead of the direct access by index.
	 */
	UPROPERTY(Transient, SaveGame)
	TArray<FScheduleEventData> EventsStack;

	/**
	 * @tparam KeyType An event that has a class that is currently being loaded or that is already loaded.
	 * @tparam ValueType A handle of the loading/loaded class.
	 */
	TMap<FScheduleEventData, TSharedPtr<FStreamableHandle>> LoadEventInstancesHandles;

	/**
	 * Temporary storage of the events stack before loading the game to end the events that are not in the stack anymore
	 * after loading the game.
	 */
	TArray<FScheduleEventData> EventsStackBeforeLoadingGame;
};