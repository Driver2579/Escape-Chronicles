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

void UBedtimeScheduleEvent::OnEventStarted(const bool bStartPaused)
{
	Super::OnEventStarted(bStartPaused);

	/**
	 * Listen for owning players to be initialized for the triggers, because the player may overlap with the trigger
	 * before it gets the owning player, which would lead to the situation where the player is overlapping with the
	 * needed trigger, but we don't check-in the player because of no owning player, which we want to avoid.
	 */
	for (TWeakObjectPtr PresenceMarkTrigger : GetPresenceMarkTriggers())
	{
#if DO_CHECK
		check(PresenceMarkTrigger.IsValid());
		check(PresenceMarkTrigger->IsA<APrisonerChamberZone>());
#endif

		const APrisonerChamberZone* PrisonerChamberZone = CastChecked<APrisonerChamberZone>(PresenceMarkTrigger);

		UPlayerOwnershipComponent* PlayerOwnershipComponent = PrisonerChamberZone->GetPlayerOwnershipComponent();

		// Wait for the owning player to be initialized on the trigger only if it isn't initialized yet
		if (!PlayerOwnershipComponent->GetOwningPlayer())
		{
			const FDelegateHandle OnOwningPlayerInitializedDelegateHandle =
				PlayerOwnershipComponent->CallOrRegister_OnOwningPlayerInitialized(
					FOnOwningPlayerInitializedDelegate::FDelegate::CreateUObject(this,
						&ThisClass::OnPrisonerChamberZoneOwningPlayerInitialized));

			// Remember the delegate handle to unsubscribe from it later
			OnOwningPlayerInitializedDelegateHandles.Add(PlayerOwnershipComponent,
				OnOwningPlayerInitializedDelegateHandle);
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UBedtimeScheduleEvent::OnPrisonerChamberZoneOwningPlayerInitialized(
	UPlayerOwnershipComponent* PlayerOwnershipComponent, const FUniquePlayerID& OwningPlayer,
	const FPlayerOwnershipComponentGroup& Group)
{
	// Retrigger an overlap for the trigger that got the owning player initialized
	TriggerBeginOverlapForOverlappingCharacters(PlayerOwnershipComponent->GetOwner());

	// Stop listening for the event and remove its handle because we don't need these anymore
	PlayerOwnershipComponent->Unregister_OnOwningPlayerInitialized(
		OnOwningPlayerInitializedDelegateHandles.FindAndRemoveChecked(PlayerOwnershipComponent));
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

void UBedtimeScheduleEvent::OnEventEnded()
{
	// We don't need to listen for the owning player initialization when the event ends, so unsubscribe from them
	UnregisterOnOwningPlayerInitializedDelegates();

	Super::OnEventEnded();
}

void UBedtimeScheduleEvent::UnregisterOnOwningPlayerInitializedDelegates()
{
	// Unsubscribe from all the events
	for (const TPair<TWeakObjectPtr<UPlayerOwnershipComponent>, FDelegateHandle>& Pair :
		OnOwningPlayerInitializedDelegateHandles)
	{
		// The component might be destroyed at this stage, so we need to check if it's valid
		if (!Pair.Key.IsValid())
		{
			continue;
		}

		const FDelegateHandle* DelegateHandle = OnOwningPlayerInitializedDelegateHandles.Find(Pair.Key);

		// Unsubscribe from the event for this component if we ever subscribed to it
		if (DelegateHandle)
		{
			Pair.Key->Unregister_OnOwningPlayerInitialized(
				OnOwningPlayerInitializedDelegateHandles.FindAndRemoveChecked(Pair.Key));
		}
	}

	// Forget about all the handles we collected
	OnOwningPlayerInitializedDelegateHandles.Empty();
}