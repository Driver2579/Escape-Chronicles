// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScheduleEventWithPresenceMark.h"
#include "BedtimeScheduleEvent.generated.h"

/**
 * TODO: Implement me
 */
UCLASS()
class ESCAPECHRONICLES_API UBedtimeScheduleEvent : public UScheduleEventWithPresenceMark
{
	GENERATED_BODY()

public:
	UBedtimeScheduleEvent();

protected:
	virtual bool CanCheckInPlayer(const AActor* PresenceMarkTrigger,
		const AEscapeChroniclesPlayerState* PlayerToCheckIn) const override;
};