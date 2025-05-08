// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/ScheduleEventWithPresenceMark.h"

#include "EngineUtils.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/TriggerBase.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UScheduleEventWithPresenceMark::OnEventStarted()
{
	Super::OnEventStarted();

#if DO_ENSURE
	ensureAlways(IsValid(PresenceMarkTriggerClass));
#endif

	// Iterate all PresenceMarkTriggers of the specified class
	for (TActorIterator It(GetWorld(), PresenceMarkTriggerClass); It; ++It)
	{
		ATriggerBase* PresenceMarkTrigger = *It;

		TArray<AActor*> OverlappingCharacters;
		PresenceMarkTrigger->GetOverlappingActors(OverlappingCharacters, AEscapeChroniclesCharacter::StaticClass());

		// Count overlaps that happened before the event started
		for (AActor* OverlappingCharacter : OverlappingCharacters)
		{
			OnPresenceMarkTriggerBeginOverlap(PresenceMarkTrigger, OverlappingCharacter);
		}

		// Listen for new overlaps
		PresenceMarkTrigger->OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnPresenceMarkTriggerBeginOverlap);

		// Add the trigger to the list of triggers to remove the delegate binding when the event ends
		PresenceMarkTriggers.Add(PresenceMarkTrigger);
	}
}

bool UScheduleEventWithPresenceMark::CanCheckInPlayer(ATriggerBase* PresenceMarkTrigger,
	AEscapeChroniclesPlayerState* PlayerToCheckIn)
{
	// Add the player to the list of checked-in players only if it's not already there
	return !CheckedInPlayers.Contains(PlayerToCheckIn->GetUniquePlayerID());
}

void UScheduleEventWithPresenceMark::OnPresenceMarkTriggerBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	// Don't check for overlaps if the event is paused
	if (IsPaused())
	{
		return;
	}

#if DO_CHECK
	check(IsValid(OverlappedActor));
	check(OverlappedActor->IsA<ATriggerBase>());
#endif

	const AEscapeChroniclesCharacter* OverlappedCharacter = Cast<AEscapeChroniclesCharacter>(OtherActor);

	// Check if the overlapped actor is a valid character
	if (!IsValid(OverlappedCharacter))
	{
		return;
	}

	// Get the PlayerState of the character to remember checked in players by their FUniquePlayerIDs
	AEscapeChroniclesPlayerState* PlayerState = OverlappedCharacter->GetPlayerStateChecked<
		AEscapeChroniclesPlayerState>();

	ATriggerBase* PresenceMarkTrigger = Cast<ATriggerBase>(OverlappedActor);

	// Add the player to the list of checked-in players only if we're allowed to
	if (CanCheckInPlayer(PresenceMarkTrigger, PlayerState))
	{
		CheckedInPlayers.Add(PlayerState->GetUniquePlayerID());

		NotifyPlayerCheckedIn(PlayerState);
	}
}

void UScheduleEventWithPresenceMark::OnEventEnded()
{
	// Unsubscribe from the overlap events
	for (TWeakObjectPtr PresenceMarkTrigger : PresenceMarkTriggers)
	{
		if (PresenceMarkTrigger.IsValid())
		{
			PresenceMarkTrigger->OnActorBeginOverlap.RemoveDynamic(this, &ThisClass::OnPresenceMarkTriggerBeginOverlap);
		}
	}

	// Forget about all the triggers we collected
	PresenceMarkTriggers.Empty();

	// Iterate all characters in the world to check which of them didn't check in during the event
	for (TActorIterator<AEscapeChroniclesCharacter> It(GetWorld()); It; ++It)
	{
		const AEscapeChroniclesCharacter* Character = *It;

		// Get the PlayerState of the character to get his FUniquePlayerID
		AEscapeChroniclesPlayerState* PlayerState = Character->GetPlayerStateChecked<AEscapeChroniclesPlayerState>();

		// Call OnPlayerMissedEvent for each player that didn't check in during the event
		if (!CheckedInPlayers.Contains(PlayerState->GetUniquePlayerID()))
		{
			NotifyPlayerMissedEvent(PlayerState);
		}
	}

	// Forget about all the checked-in players
	CheckedInPlayers.Empty();

	Super::OnEventEnded();
}