// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScheduleEventWithPresenceMark.h"
#include "MealScheduleEvent.generated.h"

class AActivitySpot;

/**
 * An event that handles upgrading the selected activity spots, meaning that gameplay effects' levels are changed during
 * this event and then reset when the event ends or paused.
 */
UCLASS()
class ESCAPECHRONICLES_API UMealScheduleEvent : public UScheduleEventWithPresenceMark
{
	GENERATED_BODY()

protected:
	virtual void OnEventStarted(const bool bStartPaused) override;

	virtual void OnEventPaused() override;
	virtual void OnEventResumed() override;

	virtual void OnEventEnded(const EScheduleEventEndReason EndReason) override;

private:
	/**
	 * A list of activity spots that should be upgraded during this event being active, meaning that their gameplay
	 * effects' levels are changed to the specified values in the map.
	 * @tparam KeyType Class of the activity spot that should be upgraded.
	 * @tparam ValueType Level the activity spot's gameplay effect should be upgraded to.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TSoftClassPtr<AActivitySpot>, uint8> ActivitySpotsClassesToUpgrade;

	/**
	 * A map of activity spots' instances that should be upgraded to the specified levels.
	 * @tparam KeyType Instance of the activity spot that should be upgraded.
	 * @tparam ValueType Level the activity spot's gameplay effect should be upgraded to.
	 */
	TMap<TWeakObjectPtr<AActivitySpot>, uint8> CachedActivitySpotsInstancesToUpgrade;

	/**
	 * Upgrades the activity spots' gameplay effects to the specified levels for them or resets them to original levels.
	 * @param bUpgrade If true, then the activity spots' gameplay effects will be upgraded. Otherwise, they will be
	 * reset to original levels.
	 */
	void SetActivitySpotsUpgraded(const bool bUpgrade);
};