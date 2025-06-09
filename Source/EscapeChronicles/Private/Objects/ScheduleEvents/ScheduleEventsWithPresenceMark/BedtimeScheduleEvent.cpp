// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/BedtimeScheduleEvent.h"

#include "EngineUtils.h"
#include "EscapeChroniclesGameplayTags.h"
#include "Actors/Door.h"
#include "Actors/Triggers/PrisonerChamberZone.h"
#include "Components/BoxComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "Objects/ScheduleEvents/AlertScheduleEvent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

UBedtimeScheduleEvent::UBedtimeScheduleEvent()
{
	// The PresenceMarkTrigger must be a PrisonerChamberZone
	SetPresenceMarkTriggerClass(APrisonerChamberZone::StaticClass());
}

void UBedtimeScheduleEvent::OnEventStarted(const bool bStartPaused)
{
	Super::OnEventStarted(bStartPaused);

	const UWorld* World = GetWorld();

#if DO_CHECK
	check(IsValid(World));
#endif

	// Cache all the doors we have to lock once the time for players to check in has passed if we should lock any
	if (!DoorsToLock.IsEmpty())
	{
		for (TActorIterator<ADoor> It(World); It; ++It)
		{
			ADoor* Door = *It;

#if DO_CHECK
			check(IsValid(Door));
#endif

			// Try to find the door in the DoorsToLock map by its key access tag
			const FLockDoorInfo* LockDoorInfo = DoorsToLock.Find(Door->GetKeyAccessTag());

			// If the door was found in the DoorsToLock map, then we have to lock it later. Cache it in this case.
			if (LockDoorInfo)
			{
				CachedDoorsInstancesToLock.Add(Door, LockDoorInfo);
			}
		}
	}

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

	AEscapeChroniclesGameState* GameState = World->GetGameState<AEscapeChroniclesGameState>();

	/**
	 * If GameState is valid, then we can listen for the game time being updated to start an alert if any player doesn't
	 * check in within the given time.
	 */
	if (ensureAlways(IsValid(GameState)))
	{
		EventStartDateTime = GameState->GetCurrentGameDateTime();

		OnCurrentGameDateTimeUpdatedDelegateHandle = GameState->OnCurrentGameDateTimeUpdated.AddUObject(this,
			&ThisClass::OnCurrentGameDateTimeUpdated);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UBedtimeScheduleEvent::OnPrisonerChamberZoneOwningPlayerInitialized(
	UPlayerOwnershipComponent* PlayerOwnershipComponent, const FUniquePlayerID& OwningPlayer,
	const FPlayerOwnershipComponentGroup& Group)
{
	/**
	 * Make sure the event didn't become paused (or wasn't initially paused when we subscribed to the event) while we
	 * were waiting for the owning player to be initialized.
	 */
	if (!IsPaused())
	{
		// Retrigger an overlap for the trigger that got the owning player initialized
		TriggerBeginOverlapForOverlappingCharacters(PlayerOwnershipComponent->GetOwner());
	}

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

void UBedtimeScheduleEvent::OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime,
	const FGameplayDateTime& NewDateTime)
{
	// Adjust the EventStartDateTime if the NewDateTime is earlier than the OldDateTime
	if (NewDateTime < OldDateTime)
	{
		EventStartDateTime -= OldDateTime - NewDateTime;

		// No need to continue because the next logic will fail anyway. It can succeed only if the time got increased.
		return;
	}

	// Check if the time for players to check in has passed
	bTimeForPlayersToCheckInPassed = NewDateTime.ToTotalMinutes() - EventStartDateTime.ToTotalMinutes() >
		TimeForPlayersToCheckIn.ToTotalMinutes();

	/**
	 * We can do the next logic only in case the event is active and not paused, and if enough time has passed since the
	 * event start.
	 */
	if (!bTimeForPlayersToCheckInPassed || IsPaused() || !IsActive())
	{
		return;
	}

	// If the time for players to check in has passed, we need to lock the specified doors
	SetDoorsLocked(true);

	// This will automatically start an alert if any player still didn't check in
	CollectPlayersThatMissedAnEvent();

	/**
	 * Unsubscribe from the event because we don't need it anymore. We can be sure the game state is valid here because
	 * this function is called by the delegate from the game state.
	 */
	GetWorld()->GetGameStateChecked<AEscapeChroniclesGameState>()->OnCurrentGameDateTimeUpdated.Remove(
		OnCurrentGameDateTimeUpdatedDelegateHandle);
}

void UBedtimeScheduleEvent::SetDoorsLocked(const bool bLockDoors)
{
	for (const auto& Pair : CachedDoorsInstancesToLock)
	{
		// Skip invalid instances in case they were destroyed
		if (!Pair.Key.IsValid())
		{
			continue;
		}

#if DO_CHECK
		check(Pair.Value);
#endif

		// Lock or unlock the door's entrance based on the bLockDoors value if we should
		if (Pair.Value->bLockEntrance)
		{
			Pair.Key->SetEnterRequiresKey(bLockDoors);
		}

		// Lock or unlock the door's exit based on the bLockDoors value if we should
		if (Pair.Value->bLockExit)
		{
			Pair.Key->SetExitRequiresKey(bLockDoors);
		}
	}
}

FBedtimeScheduleEventSaveData UBedtimeScheduleEvent::GetBedtimeScheduleEventSaveData() const
{
	return FBedtimeScheduleEventSaveData(EventStartDateTime);
}

void UBedtimeScheduleEvent::LoadBedtimeScheduleEventFromSaveData(const FBedtimeScheduleEventSaveData& SaveData)
{
#if DO_ENSURE
	ensureAlways(IsActive());
#endif

	EventStartDateTime = SaveData.EventStartDateTime;

	// Don't do anything with loaded data if the event is paused
	if (IsPaused())
	{
		return;
	}

	const AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	// Call OnCurrentGameDateTimeUpdated to update everything related to the EventStartDateTime
	if (ensureAlways(IsValid(GameState)))
	{
		OnCurrentGameDateTimeUpdated(FGameplayDateTime(), GameState->GetCurrentGameDateTime());
	}
}

void UBedtimeScheduleEvent::NotifyPlayerMissedEvent(AEscapeChroniclesPlayerState* PlayerThatMissedAnEvent)
{
	Super::NotifyPlayerMissedEvent(PlayerThatMissedAnEvent);

	// Don't do anything if there is no AlertEventData specified
	if (!ensure(AlertEventData.IsValid()))
	{
		return;
	}

#if DO_ENSURE
	ensureAlways(AlertEventData.EventTag == EscapeChroniclesGameplayTags::ScheduleEvent_Alert);
#endif

	UScheduleEventManagerComponent* ScheduleEventManagerComponent = GetScheduleEventManagerComponent();

	// Start an alert if the player missed an event but only if it isn't already started
	if (!ScheduleEventManagerComponent->IsEventInStack(AlertEventData))
	{
		// Load an alert event class synchronously because we need its instance right now
		ScheduleEventManagerComponent->PushEvent(AlertEventData, true);
	}

	const FScheduleEventData& CurrentActiveEvent = ScheduleEventManagerComponent->GetCurrentActiveEventDataChecked();

	// Add the player that missed an event to the wanted players in alert event if it's the current active one
	if (CurrentActiveEvent.EventTag == AlertEventData.EventTag)
	{
#if DO_CHECK
		check(IsValid(CurrentActiveEvent.GetEventInstance()));
		check(CurrentActiveEvent.GetEventInstance()->IsA<UAlertScheduleEvent>());
#endif

		UAlertScheduleEvent* AlertEventInstance = CastChecked<UAlertScheduleEvent>(
			CurrentActiveEvent.GetEventInstance());

		// Mark the player that missed an event as wanted in the alert event
		AlertEventInstance->AddWantedPlayer(PlayerThatMissedAnEvent);
	}
}

void UBedtimeScheduleEvent::OnEventPaused()
{
	Super::OnEventPaused();

	// We need to unlock the doors when the event is unpaused
	SetDoorsLocked(false);
}

void UBedtimeScheduleEvent::OnEventResumed()
{
	Super::OnEventResumed();

	// We need to lock the doors again if we locked them before the event was paused
	if (bTimeForPlayersToCheckInPassed)
	{
		SetDoorsLocked(true);
	}
}

void UBedtimeScheduleEvent::OnEventEnded(const EScheduleEventEndReason EndReason)
{
	/**
	 * Unlock the doors if the EndReason isn't EndPlay because there is no point of unlocking the doors if the world is
	 * about to be destroyed (yeah, that's so dramatic...).
	 */
	if (EndReason != EScheduleEventEndReason::EndPlay)
	{
		SetDoorsLocked(false);
	}

	// We don't need to listen for the owning player initialization when the event ends, so unsubscribe from them
	UnregisterOnOwningPlayerInitializedDelegates();

	// Clear the variables
	bTimeForPlayersToCheckInPassed = false;
	CachedDoorsInstancesToLock.Empty();

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	// Unsubscribe from time updates
	if (IsValid(GameState))
	{
		GameState->OnCurrentGameDateTimeUpdated.Remove(OnCurrentGameDateTimeUpdatedDelegateHandle);
	}

	Super::OnEventEnded(EndReason);
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
			Pair.Key->Unregister_OnOwningPlayerInitialized(*DelegateHandle);
		}
	}

	// Forget about all the handles we collected
	OnOwningPlayerInitializedDelegateHandles.Empty();
}
