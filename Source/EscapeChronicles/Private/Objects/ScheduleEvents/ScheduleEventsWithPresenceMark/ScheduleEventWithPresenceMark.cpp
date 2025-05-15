// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/ScheduleEventsWithPresenceMark/ScheduleEventWithPresenceMark.h"

#include "EngineUtils.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameStateBase.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UScheduleEventWithPresenceMark::OnEventStarted(const bool bStartPaused)
{
	Super::OnEventStarted(bStartPaused);

#if DO_ENSURE
	ensureAlways(!CheckedInGameplayEffectClass.IsNull());
#endif

	// Asynchronously load the gameplay effect as soon as possible to avoid delays
	LoadCheckedInGameplayEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CheckedInGameplayEffectClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnCheckedInGameplayEffectClassLoaded));

#if DO_ENSURE
	ensureAlways(IsValid(PresenceMarkTriggerClass));
#endif

	// Iterate all PresenceMarkTriggers of the specified class
	for (TActorIterator It(GetWorld(), PresenceMarkTriggerClass); It; ++It)
	{
		AActor* PresenceMarkTrigger = *It;

		// Count overlaps that happened before the event started if the event isn't started paused
		if (!bStartPaused)
		{
			TriggerBeginOverlapForOverlappingCharacters(PresenceMarkTrigger);
		}

		/**
		 * Listen for new overlaps for every component individually because child actors may want to check in the player
		 * if he overlaps with an exact component instead of the whole actor.
		 */
		PresenceMarkTrigger->ForEachComponent<UPrimitiveComponent>(false,
			[this](UPrimitiveComponent* Component)
			{
#if DO_CHECK
				check(IsValid(Component));
#endif

				Component->OnComponentBeginOverlap.AddDynamic(this,
					&ThisClass::OnPresenceMarkTriggerComponentBeginOverlap);
			});

		/**
		 * Also listen for end overlaps for the whole actor because we need to know when the player stops overlapping
		 * with it.
		 */
		PresenceMarkTrigger->OnActorEndOverlap.AddDynamic(this, &ThisClass::OnPresenceMarkTriggerEndOverlap);

		// Add the trigger to the list of triggers to remove the delegate binding when the event ends
		PresenceMarkTriggers.Add(PresenceMarkTrigger);
	}
}

void UScheduleEventWithPresenceMark::TriggerBeginOverlapForOverlappingCharacters(const AActor* PresenceMarkTrigger)
{
#if DO_CHECK
	check(IsValid(PresenceMarkTrigger));
#endif

#if DO_ENSURE
	ensureAlwaysMsgf(!IsPaused(),
		TEXT("There is no need to call this function if the event is paused. It will not do anything and will "
			"only waste performance."));
#endif

	TArray<AActor*> OverlappingCharacters;
	PresenceMarkTrigger->GetOverlappingActors(OverlappingCharacters, AEscapeChroniclesCharacter::StaticClass());

	for (AActor* OverlappingCharacter : OverlappingCharacters)
	{
#if DO_CHECK
		check(IsValid(OverlappingCharacter));
#endif

#if DO_ENSURE
		ensureAlways(OverlappingCharacter->IsA<AEscapeChroniclesCharacter>());
#endif

		OnPresenceMarkTriggerBeginOverlap(PresenceMarkTrigger, OverlappingCharacter);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UScheduleEventWithPresenceMark::OnPresenceMarkTriggerComponentBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
#if DO_CHECK
	check(IsValid(OverlappedComponent));
	check(IsValid(OverlappedComponent->GetOwner()));
#endif

	OnPresenceMarkTriggerBeginOverlap(OverlappedComponent->GetOwner(), OtherActor);
}

void UScheduleEventWithPresenceMark::OnPresenceMarkTriggerBeginOverlap(const AActor* OverlappedActor,
	AActor* OtherActor)
{
	// Don't check for overlaps if the event is paused
	if (IsPaused())
	{
		return;
	}

#if DO_CHECK
	check(IsValid(OverlappedActor));
#endif

	AEscapeChroniclesCharacter* OverlappedCharacter = Cast<AEscapeChroniclesCharacter>(OtherActor);

	// Check if the overlapped actor is a valid character
	if (!IsValid(OverlappedCharacter))
	{
		return;
	}

	// Get the PlayerState of the character to remember checked-in players by their FUniquePlayerIDs
	AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		OverlappedCharacter->GetPlayerState(), ECastCheckedType::NullAllowed);

	// If the PlayerState isn't valid, then wait for it to be initialized before checking-in the player
	if (!IsValid(PlayerState))
	{
		/**
		 * Subscribe to the PlayerState changed delegate to check in the player when the PlayerState is initialized if
		 * we didn't already subscribe before and remember the delegate handle to unsubscribe from it when the player
		 * stops overlapping with the trigger.
		 */
		if (!OverlappingPawnPlayerStateChangedDelegateHandles.Contains(OverlappedCharacter))
		{
			const FDelegateHandle DelegateHandle = OverlappedCharacter->OnPlayerStateChangedDelegate.AddUObject(this,
				&ThisClass::OnOverlappingCharacterPlayerStateChanged);

			OverlappingPawnPlayerStateChangedDelegateHandles.Add(OverlappedCharacter, DelegateHandle);
		}

		return;
	}

	TryCheckInPlayer(OverlappedActor, PlayerState);
}

void UScheduleEventWithPresenceMark::OnOverlappingCharacterPlayerStateChanged(APlayerState* NewPlayerState,
	APlayerState* OldPlayerState)
{
	// Don't do anything if the new player state is not valid
	if (!IsValid(NewPlayerState))
	{
		return;
	}

#if DO_CHECK
	check(IsValid(NewPlayerState->GetPawn()));
	check(NewPlayerState->GetPawn()->IsA<AEscapeChroniclesCharacter>());
#endif

	AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(NewPlayerState->GetPawn());

#if DO_ENSURE
	ensureAlways(IsValid(PresenceMarkTriggerClass));
#endif

	// Get all overlapping triggers to try to check in the player with all of them
	TSet<AActor*> OverlappingPresenceMarkTriggers;
	Character->GetOverlappingActors(OverlappingPresenceMarkTriggers, PresenceMarkTriggerClass);

#if DO_ENSURE
	// The character should overlap with at least one trigger by now...
	ensureAlways(!OverlappingPresenceMarkTriggers.IsEmpty());
#endif

#if DO_CHECK
	check(NewPlayerState->IsA<AEscapeChroniclesPlayerState>());
#endif

	AEscapeChroniclesPlayerState* CastedPlayerState = CastChecked<AEscapeChroniclesPlayerState>(NewPlayerState);

	// Try to check in the player with any of the overlapping triggers
	for (const AActor* OverlappingPresenceMarkTrigger : OverlappingPresenceMarkTriggers)
	{
		if (TryCheckInPlayer(OverlappingPresenceMarkTrigger, CastedPlayerState))
		{
			break;
		}
	}

	// Get the delegate handle that was used to subscribe to the PlayerState changed event and remove it from the map
	const FDelegateHandle DelegateHandle = OverlappingPawnPlayerStateChangedDelegateHandles.FindAndRemoveChecked(
		Character);

#if DO_ENSURE
	ensureAlways(DelegateHandle.IsValid());
#endif

	// Unsubscribe from the PlayerState changed event
	Character->OnPlayerStateChangedDelegate.Remove(DelegateHandle);
}

void UScheduleEventWithPresenceMark::OnPresenceMarkTriggerEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	AEscapeChroniclesCharacter* OverlappingCharacter = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(OverlappingCharacter))
	{
		return;
	}

	// Try to find the delegate handle that was used to subscribe to the PlayerState changed event if any
	const FDelegateHandle* DelegateHandle = OverlappingPawnPlayerStateChangedDelegateHandles.Find(
		OverlappingCharacter);

	// If the delegate handle is found, then unsubscribe from the event and remove it from the map
	if (DelegateHandle)
	{
#if DO_ENSURE
		ensureAlways(DelegateHandle->IsValid());
#endif

		OverlappingCharacter->OnPlayerStateChangedDelegate.Remove(*DelegateHandle);
		OverlappingPawnPlayerStateChangedDelegateHandles.Remove(OverlappingCharacter);
	}
}

bool UScheduleEventWithPresenceMark::CanCheckInPlayer(const AActor* PresenceMarkTrigger,
	const AEscapeChroniclesPlayerState* PlayerToCheckIn) const
{
	// Add the player to the list of checked-in players only if it's not already there
	return !CheckedInPlayers.Contains(PlayerToCheckIn->GetUniquePlayerID());
}

void UScheduleEventWithPresenceMark::NotifyPlayerCheckedIn(AEscapeChroniclesPlayerState* CheckedInPlayer)
{
	CheckedInPlayers.Add(CheckedInPlayer->GetUniquePlayerID());

	/**
	 * Apply the checked-in gameplay effect to the player if the gameplay effect's class is already loaded. It will be
	 * applied once it's loaded otherwise.
	 */
	if (CheckedInGameplayEffectClass.IsValid())
	{
		ApplyCheckedInGameplayEffect(CheckedInPlayer);
	}

	OnPlayerCheckedIn.Broadcast(CheckedInPlayer);
}

bool UScheduleEventWithPresenceMark::TryCheckInPlayer(const AActor* PresenceMarkTrigger,
	AEscapeChroniclesPlayerState* PlayerToCheckIn)
{
	// Add the player to the list of checked-in players only if we're allowed to
	if (!CanCheckInPlayer(PresenceMarkTrigger, PlayerToCheckIn))
	{
		return false;
	}

	NotifyPlayerCheckedIn(PlayerToCheckIn);

	return true;
}

void UScheduleEventWithPresenceMark::OnCheckedInGameplayEffectClassLoaded()
{
	// Don't do anything if there are no checked-in players
	if (CheckedInPlayers.IsEmpty())
	{
		return;
	}

	AGameStateBase* GameState = GetWorld()->GetGameState();

#if DO_CHECK
	check(IsValid(GameState));
#endif

	// Apply the checked-in gameplay effect to all players that are already checked in
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AEscapeChroniclesPlayerState* EscapeChroniclesPlayerState = Cast<AEscapeChroniclesPlayerState>(
			PlayerState);

		if (!IsValid(EscapeChroniclesPlayerState))
		{
			continue;
		}

		const FUniquePlayerID& UniquePlayerID = EscapeChroniclesPlayerState->GetUniquePlayerID();

		/**
		 * Check if UniquePlayerID of currently iterated PlayerState is valid and is within the list of checked-in
		 * players.
		 */
		const bool bPlayerCheckedIn = UniquePlayerID.IsValid() &&
			CheckedInPlayers.Contains(EscapeChroniclesPlayerState->GetUniquePlayerID());

		// If the player is checked in, then apply the gameplay effect to him
		if (bPlayerCheckedIn)
		{
			ApplyCheckedInGameplayEffect(EscapeChroniclesPlayerState);
		}
	}
}

void UScheduleEventWithPresenceMark::ApplyCheckedInGameplayEffect(const AEscapeChroniclesPlayerState* CheckedInPlayer)
{
#if DO_CHECK
	check(IsValid(CheckedInPlayer));
	check(CheckedInGameplayEffectClass.IsValid());
#endif

	UAbilitySystemComponent* AbilitySystemComponent = CheckedInPlayer->GetAbilitySystemComponent();

#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Apply the gameplay effect and remember its handle
	const FActiveGameplayEffectHandle ActiveGameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(
		CheckedInGameplayEffectClass->GetDefaultObject<UGameplayEffect>(), 1,
		AbilitySystemComponent->MakeEffectContext());

#if DO_ENSURE
	ensureAlways(ActiveGameplayEffectHandle.IsValid());
#endif

	// Remember the handle to remove the gameplay effect when the event ends
	CheckedInGameplayEffectHandles.Add(AbilitySystemComponent, ActiveGameplayEffectHandle);
}

void UScheduleEventWithPresenceMark::OnEventResumed()
{
	Super::OnEventResumed();

	/**
	 * Trigger overlaps for all characters that are currently overlapping with the triggers because we didn't count
	 * overlaps while the event was paused.
	 */
	for (TWeakObjectPtr PresenceMarkTrigger : PresenceMarkTriggers)
	{
		if (PresenceMarkTrigger.IsValid())
		{
			TriggerBeginOverlapForOverlappingCharacters(PresenceMarkTrigger.Get());
		}
	}
}

void UScheduleEventWithPresenceMark::OnEventEnded()
{
	// Unsubscribe from the overlap events
	for (TWeakObjectPtr PresenceMarkTrigger : PresenceMarkTriggers)
	{
		if (!PresenceMarkTrigger.IsValid())
		{
			continue;
		}

		PresenceMarkTrigger->ForEachComponent<UPrimitiveComponent>(false,
			[this](UPrimitiveComponent* Component)
			{
#if DO_CHECK
				check(IsValid(Component));
#endif

				Component->OnComponentBeginOverlap.RemoveDynamic(this,
					&ThisClass::OnPresenceMarkTriggerComponentBeginOverlap);
			});

		PresenceMarkTrigger->OnActorEndOverlap.RemoveDynamic(this, &ThisClass::OnPresenceMarkTriggerEndOverlap);
	}

	// Forget about all the triggers we collected
	PresenceMarkTriggers.Empty();

	/**
	 * Iterate all characters in the world to check which of them didn't check in during the event, but only if the
	 * event wasn't ended while paused.
	 */
	if (!IsPaused())
	{
		for (TActorIterator<AEscapeChroniclesCharacter> It(GetWorld()); It; ++It)
		{
			const AEscapeChroniclesCharacter* Character = *It;

			// Get the PlayerState of the character to get his FUniquePlayerID
			AEscapeChroniclesPlayerState* PlayerState = Character->GetPlayerState<AEscapeChroniclesPlayerState>();

			// Call OnPlayerMissedEvent for each valid player that didn't check in during the event
			if (IsValid(PlayerState) && !CheckedInPlayers.Contains(PlayerState->GetUniquePlayerID()))
			{
				NotifyPlayerMissedEvent(PlayerState);
			}
		}
	}

	// Forget about all the checked-in players
	CheckedInPlayers.Empty();

	/**
	 * Remove checked-in gameplay effects that were added by this event from the players that checked in during the
	 * event because event is over.
	 */
	for (const auto& GameplayEffectHandlePair : CheckedInGameplayEffectHandles)
	{
		if (GameplayEffectHandlePair.Key.IsValid() && GameplayEffectHandlePair.Value.IsValid())
		{
			GameplayEffectHandlePair.Key->RemoveActiveGameplayEffect(GameplayEffectHandlePair.Value, 1);
		}
	}

	// Clear an array of gameplay effect handles
	CheckedInGameplayEffectHandles.Empty();

	// Unload the checked-in gameplay effect class if it's loaded or cancel its loading if it's still loading
	if (LoadCheckedInGameplayEffectHandle.IsValid())
	{
		LoadCheckedInGameplayEffectHandle->CancelHandle();
		LoadCheckedInGameplayEffectHandle.Reset();
	}

	Super::OnEventEnded();
}