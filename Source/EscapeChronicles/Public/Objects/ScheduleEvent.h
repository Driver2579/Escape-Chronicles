// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Common/Structs/ScheduleEventData.h"
#include "ScheduleEvent.generated.h"

// The base class for event instances. Children of this class are what actually dictate the behavior during the event.
UCLASS(Abstract, Blueprintable)
class ESCAPECHRONICLES_API UScheduleEvent : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// ~FTickableGameObject interface
	virtual UWorld* GetTickableGameObjectWorld() const override
	{
		return GetWorld();
	}

	virtual ETickableTickType GetTickableTickType() const override
	{
		return bCanEverTick ? ETickableTickType::Conditional : ETickableTickType::Never;
	}

	virtual bool IsTickable() const override
	{
		return bTickEnabled;
	}

	// Called only if the event is active, isn't paused, and bCanEverTick is true
	virtual void Tick(float DeltaTime) override {}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UScheduleEvent, STATGROUP_Tickables);
	}
	// ~End of FTickableGameObject interface

	bool CanEverTick() const { return bCanEverTick; }
	bool IsTickEnabled() const { return bCanEverTick && bTickEnabled; }

	const FScheduleEventData& GetEventData() const { return EventData; }

	// This MUST be called when this event is created and before it's started
	void SetEventData(const FScheduleEventData& InEventData)
	{
#if DO_ENSURE
		ensureAlways(InEventData.EventClass == GetClass());
#endif

		EventData = InEventData;
	}

	bool IsActive() const { return bActive; }

	// Starts the event if it was not started before. Should be called when the event is created.
	void StartEvent();

	// Ends the event if it was started. Should be called when the event is no longer needed.
	void EndEvent();

	bool IsPaused() const { return bPaused; }

	/**
	 * Pauses the event if it was started and if it isn't already paused. Should be called when you want to override the
	 * event with another one, but don't want to completely end it yet.
	 */
	void PauseEvent();

	/**
	 * Resumes the event if it's still active and if it is currently paused. Should be called when you want to revert
	 * the state of the event to the one before it was paused.
	 */
	void ResumeEvent();

protected:
	// Can be called only from the constructor
	void SetCanEverTick(const bool bEnabled)
	{
#if DO_ENSURE
		ensureAlwaysMsgf(HasAnyFlags(RF_NeedInitialization),
			TEXT("You can only set bCanEverTick in the constructor!"));
#endif

		bCanEverTick = bEnabled;
	}

	// Called when the event is started. This is where you should start the event's logic.
	virtual void OnEventStarted() {}

	// Called when the event is ended. This is where you should clear the event's logic.
	virtual void OnEventEnded() {}

	/**
	 * Called when the event is paused. Usually, you would like to duplicate or implement the similar logic to the
	 * OnEventEnded method, but you could also implement your custom logic here (for example, only clearing some part of
	 * the event's logic but keep something passive).
	 * @remark Keep in mind that sometimes this could be called immediately after the OnEventStarted method.
	 */
	virtual void OnEventPaused() {}

	/**
	 * Called when the event is resumed. This is where you should revert your changes made in the OnEventPaused method
	 * or simply restart the event's logic like you would do in the OnEventStarted method.
	 */
	virtual void OnEventResumed() {}

private:
	// Whether this object is registered for ticking
	bool bCanEverTick = false;

	/**
	 * Whether this object currently can tick (if bCanEverTick is true as well). This is set to true while the event is
	 * active and not paused.
	 */
	bool bTickEnabled = false;

	// Event data that owns this event instance
	FScheduleEventData EventData;

	// Whether this event was started and not ended yet
	bool bActive = false;

	// Whether this event was started and is currently paused
	bool bPaused = false;
};