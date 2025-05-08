// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "ScheduleEventData.generated.h"

class UScheduleEvent;

/**
 * A struct that contains the data for events that dictate the behavior of the game. These events are mostly started by
 * the schedule, but could also be started by something else.
 * @remark It is expected that all fields in this struct are unique for each event.
 */
USTRUCT()
struct FScheduleEventData
{
	GENERATED_BODY()

	// A tag that identifies the event
	UPROPERTY(EditDefaultsOnly, SaveGame)
	FGameplayTag EventTag;

	// A class of the object dictates the behavior of the game while this event is active
	UPROPERTY(EditDefaultsOnly, SaveGame)
	TSoftClassPtr<UScheduleEvent> EventClass;

	// An actual instance of the event. Has to be created and started manually using the EventClass.
	UPROPERTY(Transient)
	TObjectPtr<UScheduleEvent> EventInstance;

	// Compares events by their tags
	bool operator==(const FScheduleEventData& Other) const
	{
		return EventTag == Other.EventTag;
	}

	// Checks if EventTag and EventClass are set
	bool IsValid() const
	{
		return EventTag.IsValid() && !EventClass.IsNull();
	}
};

// This is required to use FScheduleEventData as a key in TMap and TSet
FORCEINLINE uint32 GetTypeHash(const FScheduleEventData& ScheduleEventData)
{
	return FCrc::MemCrc32(&ScheduleEventData, sizeof(ScheduleEventData));
}