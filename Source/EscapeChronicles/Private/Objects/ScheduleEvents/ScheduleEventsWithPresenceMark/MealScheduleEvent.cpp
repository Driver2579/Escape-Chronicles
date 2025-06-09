// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/MealScheduleEvent.h"

#include "EngineUtils.h"
#include "Actors/ActivitySpot.h"

void UMealScheduleEvent::OnEventStarted(const bool bStartPaused)
{
	Super::OnEventStarted(bStartPaused);

	const UWorld* World = GetWorld();

#if DO_CHECK
	check(IsValid(World));
#endif

	// Cache all instances of specified activity spots' classes
	for (const auto& ActivitySpotClassToUpgradePair : ActivitySpotsClassesToUpgrade)
	{
		// Skip the class if it isn't loaded because in that case it's for sure isn't on the level
		if (ActivitySpotClassToUpgradePair.Key.IsNull())
		{
			continue;
		}

		// Cache all instances of an exact class and the level they should be upgraded to
		for (TActorIterator<AActivitySpot> It(World, ActivitySpotClassToUpgradePair.Key.Get()); It; ++It)
		{
#if DO_ENSURE
			ensureAlways(IsValid(*It));
#endif

			CachedActivitySpotsInstancesToUpgrade.Add(*It, ActivitySpotClassToUpgradePair.Value);
		}
	}

	/**
	 * Upgrade all activity spots we just cached already if the event wasn't started paused. They will be upgraded when
	 * the event is resumed otherwise.
	 */
	if (!bStartPaused)
	{
		SetActivitySpotsUpgraded(true);
	}
}

void UMealScheduleEvent::SetActivitySpotsUpgraded(const bool bUpgrade)
{
	for (const auto& Pair : CachedActivitySpotsInstancesToUpgrade)
	{
		// Skip invalid instances in case they were destroyed
		if (!Pair.Key.IsValid())
		{
			continue;
		}

		// Set the gameplay effect level to the specified value if upgrading was requested
		if (bUpgrade)
		{
			Pair.Key->SetEffectLevel(Pair.Value);
		}
		// Otherwise, reset the level to the original one
		else
		{
			// TODO: Uncomment

			//Pair.Key->ResetEffectLevel();
		}
	}
}

void UMealScheduleEvent::OnEventPaused()
{
	Super::OnEventPaused();

	// Reset all activity spots' gameplay effects to original levels if the event is paused
	SetActivitySpotsUpgraded(false);
}

void UMealScheduleEvent::OnEventResumed()
{
	// Upgrade all activity spots' gameplay effects to the specified levels once the event is resumed
	SetActivitySpotsUpgraded(true);

	Super::OnEventResumed();
}

void UMealScheduleEvent::OnEventEnded(const EScheduleEventEndReason EndReason)
{
	// Reset all activity spots' gameplay effects to original levels once the event is ended
	SetActivitySpotsUpgraded(false);

	// Clear the cached instances of activity spots because we don't need them anymore
	CachedActivitySpotsInstancesToUpgrade.Empty();

	Super::OnEventEnded(EndReason);
}