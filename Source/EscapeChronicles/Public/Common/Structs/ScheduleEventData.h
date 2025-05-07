// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTagContainer.h"
#include "ScheduleEventData.generated.h"

class UScheduleEvent;

USTRUCT()
struct FScheduleEventData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, SaveGame)
	FGameplayTag EventTag;

	UPROPERTY(EditDefaultsOnly, SaveGame)
	TSoftClassPtr<UScheduleEvent> EventClass;

	UPROPERTY(Transient)
	TObjectPtr<UScheduleEvent> EventInstance;

	bool operator==(const FScheduleEventData& Other) const
	{
		return EventTag == Other.EventTag;
	}

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