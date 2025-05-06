// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Common/Structs/ScheduleEventData.h"
#include "ScheduleEvent.generated.h"

/**
 * 
 */
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

	virtual void Tick(float DeltaTime) override {}
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

	virtual void StartEvent();
	virtual void EndEvent();

	bool IsPaused() const { return bPaused; }

	virtual void PauseEvent();
	virtual void ResumeEvent();

protected:
	void SetCanEverTick(const bool bEnabled)
	{
#if DO_ENSURE
		ensureAlwaysMsgf(HasAnyFlags(RF_NeedInitialization),
			TEXT("You can only set bCanEverTick in the constructor"));
#endif

		bCanEverTick = bEnabled;
	}

	virtual void OnEventStarted() {}
	virtual void OnEventEnded() {}

	virtual void OnEventPaused() {}
	virtual void OnEventResumed() {}

private:
	// Whether this object is registered for ticking
	bool bCanEverTick = false;

	// Whether this object currently can tick
	bool bTickEnabled = false;

	FScheduleEventData EventData;

	bool bActive = false;
	bool bPaused = false;
};