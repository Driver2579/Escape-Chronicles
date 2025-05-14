// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/BedtimeScheduleEvent.h"

#include "Actors/Triggers/PrisonerChamberZone.h"
#include "Components/BoxComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

UBedtimeScheduleEvent::UBedtimeScheduleEvent()
{
	// The PresenceMarkTrigger must be a PrisonerChamberZone
	SetPresenceMarkTriggerClass(APrisonerChamberZone::StaticClass());
}

bool UBedtimeScheduleEvent::CanCheckInPlayer(const AActor* PresenceMarkTrigger,
	const AEscapeChroniclesPlayerState* PlayerToCheckIn) const
{
	if (!Super::CanCheckInPlayer(PresenceMarkTrigger, PlayerToCheckIn))
	{
		return false;
	}

#if DO_CHECK
	// The PresenceMarkTrigger must be a PrisonerChamberZone
	check(IsValid(PresenceMarkTrigger));
	check(PresenceMarkTrigger->IsA<APrisonerChamberZone>());

	check(IsValid(PlayerToCheckIn));
#endif

	const APrisonerChamberZone* PrisonerChamberZone = CastChecked<APrisonerChamberZone>(PresenceMarkTrigger);

	const FUniquePlayerID* PrisonerChamberZoneOwningPlayer =
		PrisonerChamberZone->GetPlayerOwnershipComponent()->GetOwningPlayer();

	// Check if any player even owns a prisoner's chamber zone
	if (!PrisonerChamberZoneOwningPlayer)
	{
		return false;
	}

	const APawn* PawnToCheckIn = PlayerToCheckIn->GetPawn();

#if DO_CHECK
	check(IsValid(PawnToCheckIn));
#endif

	/**
	 * Check if the player is the owner of the prisoner's chamber zone. He can only check in if he is the owner. And
	 * check if the player is overlapping with the inner zone of the prisoner's chamber zone. Outer zone isn't enough to
	 * check in.
	 */
	return *PrisonerChamberZoneOwningPlayer == PlayerToCheckIn->GetUniquePlayerID() &&
		PrisonerChamberZone->GetInnerZoneBoxComponent()->IsOverlappingActor(PawnToCheckIn);
}