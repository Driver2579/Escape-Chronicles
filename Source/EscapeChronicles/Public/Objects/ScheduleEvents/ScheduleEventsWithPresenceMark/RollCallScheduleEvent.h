// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/ScheduleEventWithPresenceMark.h"
#include "RollCallScheduleEvent.generated.h"

// An event that starts an alert if any players miss the event
UCLASS()
class ESCAPECHRONICLES_API URollCallScheduleEvent : public UScheduleEventWithPresenceMark
{
	GENERATED_BODY()

protected:
	virtual void NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent) override;

private:
	// An event to start if any players miss this event. Expected to be an alert event.
	UPROPERTY(EditDefaultsOnly, Category="Alert")
	FScheduleEventData AlertEventData;
};