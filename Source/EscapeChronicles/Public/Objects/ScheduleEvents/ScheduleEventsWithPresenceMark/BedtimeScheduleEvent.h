// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScheduleEventWithPresenceMark.h"
#include "Common/Structs/SaveData/BedtimeScheduleEventSaveData.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "BedtimeScheduleEvent.generated.h"

/**
 * An event that handles the players checking in only in their prisoner chambers, filtering the chambers of other
 * prisoners. This event has a time limit for the players to check in. If they don't check in within this time, then an
 * alert event will be started.
 */
UCLASS()
class ESCAPECHRONICLES_API UBedtimeScheduleEvent : public UScheduleEventWithPresenceMark
{
	GENERATED_BODY()

public:
	UBedtimeScheduleEvent();

	// Should be used when the game is saved
	FBedtimeScheduleEventSaveData GetBedtimeScheduleEventSaveData() const;

	// Should be called when the game is loaded
	void LoadBedtimeScheduleEventFromSaveData(const FBedtimeScheduleEventSaveData& SaveData);

	bool HasTimeForPlayersToCheckInPassed() const { return bTimeForPlayersToCheckInPassed; }

protected:
	virtual void OnEventStarted(const bool bStartPaused) override;

	virtual bool CanCheckInPlayer(const AActor* PresenceMarkTrigger,
		const AEscapeChroniclesPlayerState* PlayerToCheckIn) const override;

	// TODO: Close the door on lock when the player is checked in and update documentation

	virtual void OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime,
		const FGameplayDateTime& NewDateTime);

	virtual void NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent) override;

	virtual void OnEventEnded(const EScheduleEventEndReason EndReason) override;

private:
	TMap<TWeakObjectPtr<UPlayerOwnershipComponent>, FDelegateHandle> OnOwningPlayerInitializedDelegateHandles;

	void OnPrisonerChamberZoneOwningPlayerInitialized(UPlayerOwnershipComponent* PlayerOwnershipComponent,
		const FUniquePlayerID& OwningPlayer, const FPlayerOwnershipComponentGroup& Group);

	FGameplayDateTime EventStartDateTime;
	FDelegateHandle OnCurrentGameDateTimeUpdatedDelegateHandle;

	/**
	 * The time players have to check in after the event started. If some of the real players don't check in within this
	 * time, an alert will be started for them.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Alert")
	FGameplayTime TimeForPlayersToCheckIn = FGameplayTime(1, 0);

	// Whether the time for players to check in has passed
	bool bTimeForPlayersToCheckInPassed = false;

	// An event to start if the players don't check in within the time limit. Expected to be an alert event.
	UPROPERTY(EditDefaultsOnly, Category="Alert")
	FScheduleEventData AlertEventData;

	void UnregisterOnOwningPlayerInitializedDelegates();
};