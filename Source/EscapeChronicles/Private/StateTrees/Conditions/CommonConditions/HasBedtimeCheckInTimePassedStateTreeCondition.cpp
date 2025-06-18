// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/HasBedtimeCheckInTimePassedStateTreeCondition.h"

#include "StateTreeExecutionContext.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/BedtimeScheduleEvent.h"

bool FHasBedtimeCheckInTimePassedStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const UWorld* World = Context.GetWorld();

	// Return false if the world is invalid. If bInvert is true, then true will be returned.
	if (!ensureAlways(IsValid(World)))
	{
		return bInvert;
	}

	const AEscapeChroniclesGameMode* GameMode = World->GetAuthGameMode<AEscapeChroniclesGameMode>();

	// Return false if the GameMode is invalid. If bInvert is true, then true will be returned.
	if (!ensureAlways(IsValid(GameMode)))
	{
		return bInvert;
	}

	const UBedtimeScheduleEvent* BedtimeEventInstance = Cast<UBedtimeScheduleEvent>(
		GameMode->GetScheduleEventManagerComponent()->GetCurrentActiveEventData().GetEventInstance());

	/**
	 * Return false if the current active event isn't a bedtime event or if its instance isn't valid yet. If bInvert is
	 * true, then true will be returned.
	 */
	if (!IsValid(BedtimeEventInstance))
	{
		return bInvert;
	}

	// Return the final result based on whether the time for players to check in has passed and invert it if needed
	return BedtimeEventInstance->HasTimeForPlayersToCheckInPassed() ^ bInvert;
}