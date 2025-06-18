// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScheduleEventWithPresenceMark.h"
#include "Common/Structs/SaveData/BedtimeScheduleEventSaveData.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "BedtimeScheduleEvent.generated.h"

class ADoor;

// Information about how the door should be locked during the UBedtimeScheduleEvent
USTRUCT()
struct FLockDoorInfo
{
	GENERATED_BODY()

	// Whether the door's entrance should be locked
	UPROPERTY(EditAnywhere)
	bool bLockEntrance = false;

	// Whether the door's exit should be locked
	UPROPERTY(EditAnywhere)
	bool bLockExit = true;
};

/**
 * An event that handles the players checking in only in their prisoner chambers, filtering the chambers of other
 * prisoners. This event has a time limit for the players to check in. If they don't check in within this time, then an
 * alert event will be started. Also, once this time limit ends, all the specified doors will be closed until the event
 * ends or paused.
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

	virtual void OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime,
		const FGameplayDateTime& NewDateTime);

	virtual void NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent) override;

	virtual void OnEventPaused() override;
	virtual void OnEventResumed() override;

	virtual void OnEventEnded(const EScheduleEventEndReason EndReason) override;

private:
	TMap<TWeakObjectPtr<UPlayerOwnershipComponent>, FDelegateHandle> OnOwningPlayerInitializedDelegateHandles;

	void OnPrisonerChamberZoneOwningPlayerInitialized(UPlayerOwnershipComponent* PlayerOwnershipComponent,
		const FUniquePlayerID& OwningPlayer, const FPlayerOwnershipComponentGroup& Group);

	FGameplayDateTime EventStartDateTime;
	FDelegateHandle OnCurrentGameDateTimeUpdatedDelegateHandle;

	/**
	 * The time players have to check in after the event started. Once this time ends, all specified doors in the
	 * DoorsToLock list will be locked. If some of the real players don't check in within this time, an alert will be
	 * started for them.
	 */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTime TimeForPlayersToCheckIn = FGameplayTime(1, 0);

	/**
	 * Sets the EventStartDateTime to the current GameDateTime and registers the OnCurrentGameDateTimeUpdated function
	 * if it isn't already. If bReset is true, then it will also clear the list of checked-in players and the list of
	 * players that missed the event and will recollect the checked-in players from the PresenceMarkTriggers.
	 */
	void PostEventStarted(const bool bReset = false);

	// Whether the time for players to check in has passed
	bool bTimeForPlayersToCheckInPassed = false;

	/**
	 * A list of doors that should be locked once the time for players to check in has passed.
	 * @tparam KeyType A key access tag that the door requires to be opened when it isn't locked.
	 * @tparam ValueType An information about how exactly the door should be locked.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FLockDoorInfo> DoorsToLock;

	/**
	 * Instances of doors that should be locked once the time for players to check in has passed.
	 * @tparam KeyType An instance of the door on the scene.
	 * @tparam ValueType A reference to the information about how exactly the door should be locked.
	 */
	TMap<TWeakObjectPtr<ADoor>, const FLockDoorInfo*> CachedDoorsInstancesToLock;

	// Locks or unlocks all the doors in the CachedDoorsInstancesToLock map based on the FLockDoorInfo
	void SetDoorsLocked(const bool bLockDoors);

	// An event to start if the players don't check in within the time limit. Expected to be an alert event.
	UPROPERTY(EditDefaultsOnly, Category="Alert")
	FScheduleEventData AlertEventData;

	void UnregisterOnOwningPlayerInitializedDelegates();
};