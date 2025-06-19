// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/ScheduleEvents/ScheduleEvent.h"
#include "Common/Structs/UniquePlayerID.h"
#include "ActiveGameplayEffectHandle.h"
#include "ScheduleEventWithPresenceMark.generated.h"

class AEscapeChroniclesCharacter;
class UAbilitySystemComponent;
class UGameplayEffect;
class AEscapeChroniclesPlayerState;

/**
 * The base class for events that require a presence mark from players by colliding with one of the PresenceMarkTriggers
 * of a specified class.
 */
UCLASS(Abstract)
class ESCAPECHRONICLES_API UScheduleEventWithPresenceMark : public UScheduleEvent
{
	GENERATED_BODY()

public:
	bool IsPlayerCheckIn(const AEscapeChroniclesPlayerState* PlayerToCheck) const;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerCheckedInDelegate, AEscapeChroniclesPlayerState* CheckedInPlayer);

	FOnPlayerCheckedInDelegate OnPlayerCheckedIn;

	// Should be used when the game is saved
	FScheduleEventWithPresenceMarkSaveData GetScheduleEventWithPresenceMarkSaveData() const;

	// Should be called when the game is loaded
	void LoadScheduleEventWithPresenceMarkFromSaveData(const FScheduleEventWithPresenceMarkSaveData& SaveData);

protected:
	// Can be called only from the constructor
	void SetPresenceMarkTriggerClass(const TSubclassOf<AActor>& NewPresenceMarkTriggerClass)
	{
#if DO_ENSURE
			ensureAlwaysMsgf(HasAnyFlags(RF_NeedInitialization),
				TEXT("You can only set PresenceMarkTriggerClass in the constructor!"));
#endif

			PresenceMarkTriggerClass = NewPresenceMarkTriggerClass;
	}

	virtual void OnEventStarted(const bool bStartPaused) override;

	const TArray<TWeakObjectPtr<AActor>>& GetPresenceMarkTriggers() const { return PresenceMarkTriggers; }

	/**
	 * Collects all players that are currently overlapping with the given trigger and calls
	 * OnPresenceMarkTriggerComponentBeginOverlap for them.
	 */
	void TriggerBeginOverlapForOverlappingCharacters(const AActor* PresenceMarkTrigger);

	/**
	 * @return Whether the player can be marked as checked-in when overlapping with the PresenceMarkTrigger
	 */
	virtual bool CanCheckInPlayer(const AActor* PresenceMarkTrigger,
		const AEscapeChroniclesPlayerState* PlayerToCheckIn) const;

	/**
	 * Called when a player checks in during the event (overlaps with the PresenceMarkTrigger). Adds the player to the
	 * list of checked-in players, applies the checked-in gameplay effect to the player if the gameplay effect is loaded
	 * already (it will be applied once loaded otherwise), and broadcasts the delegate that the player has checked in.
	 */
	virtual void NotifyPlayerCheckedIn(AEscapeChroniclesPlayerState* CheckedInPlayer);

	virtual void OnEventResumed() override;

	virtual void OnEventEnded(const EScheduleEventEndReason EndReason) override;

	/**
	 * Calls NotifyPlayerMissedEvent for all players that didn't check in during the event (didn't overlap with the
	 * PresenceMarkTrigger). Called automatically when the event ends if it isn't paused, but you can call it manually
	 * sooner (just keep in mind that it doesn't prevent the function from being called again later).
	 * @param bForceLoadGameplayEffect If this is set to true, then MissedPlayerGameplayEffectClass will be immediately
	 * loaded synchronously and applied to all missed players after that. Otherwise, it will be loaded asynchronously
	 * and applied to all missed players once the loading is finished.
	 */
	void CollectPlayersThatMissedAnEvent(const bool bForceLoadGameplayEffect = false);

	/**
	 * Checks if we can call NotifyPlayerMissedEvent for the given player by checking if the player didn't check in, the
	 * player wasn't yet marked as missed, and the player is a prisoner.
	 */
	virtual bool CanPlayerMissEvent(const AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent) const;

	// Called by CollectPlayersThatMissedAnEvent for every player that passed the CanPlayerMissEvent check
	virtual void NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent);

	// Clears the list of players that have checked in during the event
	void ClearCheckedInPlayers();

	// Clears the list of players that missed an event
	void ClearPlayersThatMissedAnEvent();

private:
	// The class of the trigger that is used to check in players
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> PresenceMarkTriggerClass;

	// The list of triggers that are used to check in players
	TArray<TWeakObjectPtr<AActor>> PresenceMarkTriggers;

	UFUNCTION()
	void OnPresenceMarkTriggerComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * This function only exists for TriggerBeginOverlapForOverlappingCharacters function to work with the whole actor
	 * instead of checking for overlaps with every component because there is no need to do it there. It contains an
	 * actual implementation of the OnPresenceMarkTriggerComponentBeginOverlap function.
	 */
	void OnPresenceMarkTriggerBeginOverlap(const AActor* OverlappedActor, AActor* OtherActor);

	TMap<TWeakObjectPtr<APawn>, FDelegateHandle> OverlappingPawnPlayerStateChangedDelegateHandles;

	void OnOverlappingCharacterPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState);

	UFUNCTION()
	void OnPresenceMarkTriggerEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	// Calls NotifyPlayerCheckedIn for the player if CanCheckInPlayer returns true
	bool TryCheckInPlayer(const AActor* PresenceMarkTrigger, AEscapeChroniclesPlayerState* PlayerToCheckIn);

	// The list of players that checked in during the event
	TSet<FUniquePlayerID> CheckedInPlayers;

	/**
	 * Gameplay effect that is applied to the player when he check in and removed when the event is ended. Must add the
	 * Status_CheckedIn tag to the player.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> CheckedInGameplayEffectClass;

	TSharedPtr<FStreamableHandle> LoadCheckedInGameplayEffectHandle;

	void OnCheckedInGameplayEffectClassLoaded();

	void ApplyCheckedInGameplayEffect(const AEscapeChroniclesPlayerState* CheckedInPlayer);

	// The map of checked-in gameplay effect handles for each checked-in player
	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> CheckedInGameplayEffectHandles;

	// Players that didn't check in within the given time during the event
	TSet<FUniquePlayerID> PlayersThatMissedAnEvent;

	// Gameplay effect that is applied to the player if he misses the event
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> MissedPlayerGameplayEffectClass;

	TSharedPtr<FStreamableHandle> LoadMissedPlayerGameplayEffectClassHandle;

	// Applies the missed player gameplay effect to all players that missed the event
	void OnMissedPlayerGameplayEffectClassLoaded() const;

	void ApplyMissedPlayerGameplayEffect(const AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent) const;
};