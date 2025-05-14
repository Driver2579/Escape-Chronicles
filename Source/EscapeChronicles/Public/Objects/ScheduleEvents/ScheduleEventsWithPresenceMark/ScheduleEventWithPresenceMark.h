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
	// Should be used when the game is saved
	const TArray<FUniquePlayerID>& GetCheckedInPlayers() const { return CheckedInPlayers; }

	// Should be called when the game is loaded
	void SetCheckedInPlayers(const TArray<FUniquePlayerID>& InCheckedInPlayers)
	{
		CheckedInPlayers = InCheckedInPlayers;
	}

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerCheckedInDelegate, AEscapeChroniclesPlayerState* CheckedInPlayer);

	FOnPlayerCheckedInDelegate OnPlayerCheckedIn;

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

	// Whether the player can be marked as checked-in when overlapping with the PresenceMarkTrigger
	virtual bool CanCheckInPlayer(const AActor* PresenceMarkTrigger,
		const AEscapeChroniclesPlayerState* PlayerToCheckIn) const;

	/**
	 * Called when a player checks in during the event (overlaps with the PresenceMarkTrigger). Adds the player to the
	 * list of checked-in players, applies the checked-in gameplay effect to the player if the gameplay effect is loaded
	 * already (it will be applied once loaded otherwise), and broadcasts the delegate that the player has checked in.
	 */
	virtual void NotifyPlayerCheckedIn(AEscapeChroniclesPlayerState* CheckedInPlayer);

	virtual void OnEventResumed() override;

	virtual void OnEventEnded() override;

	/**
	 * Called at the end of the event for each player that didn't check in during the event (didn't overlap with the
	 * PresenceMarkTrigger) if the event isn't paused during the end.
	 */
	virtual void NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent) {}

private:
	// The class of the trigger that is used to check in players
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> PresenceMarkTriggerClass;

	// The list of triggers that are used to check in players
	TArray<TWeakObjectPtr<AActor>> PresenceMarkTriggers;

	/**
	 * Collects all players that are currently overlapping with the given trigger and calls
	 * OnPresenceMarkTriggerComponentBeginOverlap for them.
	 */
	void TriggerBeginOverlapForOverlappingCharacters(const AActor* PresenceMarkTrigger);

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
	TArray<FUniquePlayerID> CheckedInPlayers;

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
};