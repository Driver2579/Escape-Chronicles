// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Structs/ScheduleEventData.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/GameplayTime.h"
#include "ScheduleEventManagerComponent.generated.h"

struct FStreamableHandle;

// TODO: Add docs to different classes
// TODO: Connect the system with GameState

UCLASS(ClassGroup=(Custom))
class ESCAPECHRONICLES_API UScheduleEventManagerComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UScheduleEventManagerComponent();

	const FScheduleEventData& GetCurrentScheduledEventData() const
	{
#if DO_CHECK
		check(!EventsStack.IsEmpty());
#endif

		return EventsStack[0];
	}

	const FScheduleEventData& GetCurrentActiveEventData() const
	{
#if DO_CHECK
		check(!EventsStack.IsEmpty());
#endif

		return EventsStack.Last();
	}

	void PushEvent(const FScheduleEventData& EventData);
	void RemoveEvent(const FGameplayTag& EventTag);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnCurrentTimeUpdated(const FGameplayTime& GameplayTime);

	virtual void OnPostLoadObject() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTime, FScheduleEventData> ScheduleEvents;

	void CreateEventInstanceAndStartIt(const FScheduleEventData& EventData);
	void OnEventClassLoaded(FScheduleEventData EventDataCopy);

	void RemoveEventByIndex(const int32 Index, const EAllowShrinking AllowShrinking = EAllowShrinking::Yes,
		const bool bStartOrUnpauseLastEvent = true);

	static void EndEvent(FScheduleEventData& EventData);

	UPROPERTY(Transient, SaveGame)
	TArray<FScheduleEventData> EventsStack;
};