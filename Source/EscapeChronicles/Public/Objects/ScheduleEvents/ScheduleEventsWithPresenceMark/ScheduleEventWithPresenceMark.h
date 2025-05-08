// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/ScheduleEvents/ScheduleEvent.h"
#include "Common/Structs/UniquePlayerID.h"
#include "ScheduleEventWithPresenceMark.generated.h"

class AEscapeChroniclesPlayerState;
class ATriggerBase;

/**
 * The base class for events that require a presence mark from players by colliding with one of the PresenceMarkTriggers
 * of a specified class.
 */
UCLASS(Abstract)
class ESCAPECHRONICLES_API UScheduleEventWithPresenceMark : public UScheduleEvent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerCheckedInDelegate, AEscapeChroniclesPlayerState* CheckedInPlayer);

	FOnPlayerCheckedInDelegate OnPlayerCheckedIn;

protected:
	virtual void OnEventStarted() override;

	// Whether the player can be marked as checked-in when overlapping with the PresenceMarkTrigger
	virtual bool CanCheckInPlayer(ATriggerBase* PresenceMarkTrigger, AEscapeChroniclesPlayerState* PlayerToCheckIn);

	// Called when a player checks in during the event (overlaps with the PresenceMarkTrigger)
	virtual void NotifyPlayerCheckedIn(AEscapeChroniclesPlayerState* CheckedInPlayer)
	{
		OnPlayerCheckedIn.Broadcast(CheckedInPlayer);
	}

	virtual void OnEventEnded() override;

	/**
	 * Called at the end of the event for each player that didn't check in during the event (didn't overlap with the
	 * PresenceMarkTrigger)
	 */
	virtual void NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent) {}

private:
	// The class of the trigger that is used to check in players
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATriggerBase> PresenceMarkTriggerClass;

	// The list of triggers that are used to check in players
	TArray<TWeakObjectPtr<ATriggerBase>> PresenceMarkTriggers;

	UFUNCTION()
	void OnPresenceMarkTriggerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	// The list of players that checked in during the event
	TArray<FUniquePlayerID> CheckedInPlayers;
};