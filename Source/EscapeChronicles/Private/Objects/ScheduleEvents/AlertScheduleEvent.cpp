// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/ScheduleEvents/AlertScheduleEvent.h"

#include "DynamicMeshActor.h"
#include "EngineUtils.h"
#include "Components/ActorComponents/DestructibleComponent.h"
#include "Engine/AssetManager.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UAlertScheduleEvent::OnEventStarted(const bool bStartPaused)
{
	Super::OnEventStarted(bStartPaused);

	AEscapeChroniclesGameMode* GameMode = GetWorld()->GetAuthGameMode<AEscapeChroniclesGameMode>();

	// Subscribe to events to know when the player joins or leaves the game
	if (ensureAlways(IsValid(GameMode)))
	{
		OnPlayerOrBotInitializedDelegateHandle = GameMode->OnPlayerOrBotInitialized.AddUObject(this,
			&ThisClass::OnPlayerOrBotInitialized);

		OnPlayerOrBotLogoutDelegateHandle = GameMode->OnPlayerOrBotLogout.AddUObject(this,
			&ThisClass::OnPlayerOrBotLogout);
	}
}

void UAlertScheduleEvent::AddWantedPlayer(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

#if DO_ENSURE
	// Make sure we don't add the same PlayerState twice
	ensureAlways(!WantedPlayersInGame.Contains(PlayerState));
#endif

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

#if DO_ENSURE
	ensureAlways(PlayerState->GetUniquePlayerID().IsValid());
#endif

	/**
	 * Add the player to the wanted players list if it isn't already there. We assume that we don't add players to
	 * WantedPlayersInGame array twice, but can call this function for the player that is already in the WantedPlayers
	 * array but isn't in the WantedPlayersInGame array yet (for example, if the player marked as wanted just joined the
	 * game).
	 */
	if (!WantedPlayers.Contains(UniquePlayerID))
	{
		WantedPlayers.Add(PlayerState->GetUniquePlayerID());
	}

	WantedPlayersInGame.Add(PlayerState);

	// Apply the gameplay effect to the player if the class is already loaded
	if (WantedGameplayEffectClass.IsValid())
	{
		ApplyWantedGameplayEffect(PlayerState);
	}
	/**
	 * Otherwise, load the gameplay effect class and apply it when it's ready if we haven't already requested the
	 * loading.
	 */
	else if (!LoadWantedGameplayEffectClassHandle.IsValid())
	{
#if DO_ENSURE
		ensureAlways(!WantedGameplayEffectClass.IsNull());
#endif

		LoadWantedGameplayEffectClassHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			WantedGameplayEffectClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::ApplyWantedGameplayEffectToAllWantedPlayers));
	}
}

void UAlertScheduleEvent::RemoveWantedPlayer(AEscapeChroniclesPlayerState* PlayerState,
	const EAllowShrinking AllowShrinking, const bool bEndEventIfEmpty)
{
	// Find an index of the given PlayerState in the WantedPlayersInGame array
	const int32 WantedPlayerIndex = WantedPlayersInGame.IndexOfByKey(PlayerState);

	// If no player was found, then we don't need to do anything
	if (WantedPlayerIndex == INDEX_NONE)
	{
		return;
	}

	// Remove the player from the WantedPlayersInGame array
	WantedPlayersInGame.RemoveAt(WantedPlayerIndex, AllowShrinking);

	/**
	 * We need to make sure the given PlayerState is valid before we do anything with it. This function might be called
	 * with NULL player passed, which would simply mean that we need to remove the null player from the
	 * WantedPlayersInGame array (which we already did).
	 */
	if (ensureAlways(IsValid(PlayerState)))
	{
#if DO_ENSURE
		ensureAlways(PlayerState->GetUniquePlayerID().IsValid());
#endif

#if DO_CHECK
		check(WantedPlayers.Contains(PlayerState->GetUniquePlayerID()));
#endif

		// Remove the player from the WantedPlayers array
		WantedPlayers.RemoveAt(WantedPlayers.IndexOfByKey(PlayerState->GetUniquePlayerID()), AllowShrinking);

		// Remove the gameplay effect from the player if it was applied
		RemoveWantedGameplayEffect(PlayerState);
	}

	// If there are no wanted players left in the game, then remove and end this event if we're allowed to
	if (bEndEventIfEmpty && WantedPlayersInGame.IsEmpty())
	{
		GetScheduleEventManagerComponent()->RemoveEvent(GetEventData().EventTag);
	}
}

void UAlertScheduleEvent::SetWantedPlayers(const TSet<AEscapeChroniclesPlayerState*>& NewWantedPlayers)
{
	// Remove all players that are in the game but not in the new wanted players list
	for (int32 i = WantedPlayersInGame.Num() - 1; i >= 0; --i)
	{
		AEscapeChroniclesPlayerState* WantedPlayerInGame = WantedPlayersInGame[i].Get();

		if (!NewWantedPlayers.Contains(WantedPlayerInGame))
		{
			RemoveWantedPlayer(WantedPlayerInGame, EAllowShrinking::No, false);
		}
	}

	// Remove all other players from the wanted players array that are not in the new wanted players list
	for (int32 i = WantedPlayers.Num() - 1; i >= 0; --i)
	{
		const FUniquePlayerID& WantedPlayer = WantedPlayers[i];

		bool bRemovePlayer = true;

		// Check if the player is in the new wanted players list
		for (const AEscapeChroniclesPlayerState* NewWantedPlayer : NewWantedPlayers)
		{
#if DO_ENSURE
			ensureAlways(NewWantedPlayer->GetUniquePlayerID().IsValid());
#endif

			if (NewWantedPlayer->GetUniquePlayerID() == WantedPlayer)
			{
				bRemovePlayer = false;

				break;
			}
		}

		// Remove the player from the wanted players array if it isn't in the new wanted players list
		if (bRemovePlayer)
		{
			WantedPlayers.RemoveAt(i, EAllowShrinking::No);
		}
	}

	// Shrink arrays to remove all empty slots after removing players from them
	WantedPlayersInGame.Shrink();
	WantedPlayers.Shrink();

	// Add all new players to the wanted players list
	for (AEscapeChroniclesPlayerState* PlayerState : NewWantedPlayers)
	{
		if (!WantedPlayersInGame.Contains(PlayerState))
		{
			AddWantedPlayer(PlayerState);
		}
	}

	// If there are no wanted players left in the game, then remove and end this event
	if (WantedPlayersInGame.IsEmpty())
	{
		GetScheduleEventManagerComponent()->RemoveEvent(GetEventData().EventTag);
	}
}

void UAlertScheduleEvent::SetWantedPlayers(const TSet<FUniquePlayerID>& NewWantedPlayers)
{
	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	// Don't do anything if GameState isn't valid
	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AEscapeChroniclesPlayerState* CastedPlayerState = Cast<AEscapeChroniclesPlayerState>(PlayerState);

		if (!IsValid(CastedPlayerState))
		{
			continue;
		}

		const FUniquePlayerID& UniquePlayerID = CastedPlayerState->GetUniquePlayerID();

		/**
		 * If UniquePlayerID of the current iterated PlayerState is in the NewWantedPlayers, then add the player to the
		 * wanted players list if he isn't already in the WantedPlayersInGame array.
		 */
		if (NewWantedPlayers.Contains(UniquePlayerID))
		{
			if (!WantedPlayersInGame.Contains(CastedPlayerState))
			{
				AddWantedPlayer(CastedPlayerState);
			}
		}
		/**
		 * Otherwise, if the player isn't in the NewWantedPlayers but is in the WantedPlayers, then remove the player
		 * from wanted players.
		 */
		else if (WantedPlayers.Contains(UniquePlayerID))
		{
			RemoveWantedPlayer(CastedPlayerState, EAllowShrinking::No, false);
		}
	}

	// Set the whole NewWantedPlayers set to the WantedPlayers array to update other players that are not in the game
	WantedPlayers = NewWantedPlayers.Array();

	// Shrink the array to remove all empty slots after removing players from them
	WantedPlayersInGame.Shrink();

	// If there are no wanted players left in the game, then remove and end this eventaaa
	if (WantedPlayersInGame.IsEmpty())
	{
		GetScheduleEventManagerComponent()->RemoveEvent(GetEventData().EventTag);
	}
}

void UAlertScheduleEvent::ApplyWantedGameplayEffectToAllWantedPlayers()
{
#if DO_CHECK
	check(WantedGameplayEffectClass.IsValid());
#endif

	// Apply the gameplay effect to all players that are in the game
	for (TWeakObjectPtr WantedPlayerInGame : WantedPlayersInGame)
	{
		if (ensureAlways(WantedPlayerInGame.IsValid()))
		{
			ApplyWantedGameplayEffect(WantedPlayerInGame.Get());
		}
	}
}

void UAlertScheduleEvent::RemoveWantedGameplayEffectFromAllWantedPlayers()
{
	// Remove the gameplay effect from all players that are in the game
	for (TWeakObjectPtr WantedPlayerInGame : WantedPlayersInGame)
	{
		if (ensureAlways(WantedPlayerInGame.IsValid()))
		{
			RemoveWantedGameplayEffect(WantedPlayerInGame.Get());
		}
	}
}

void UAlertScheduleEvent::ApplyWantedGameplayEffect(const AEscapeChroniclesPlayerState* PlayerState)
{
	// Don't apply the gameplay effect if the event is paused
	if (IsPaused())
	{
		return;
	}

#if DO_CHECK
	check(IsValid(PlayerState));
	check(WantedGameplayEffectClass.IsValid());
#endif

#if DO_ENSURE
	ensureAlways(WantedPlayersInGame.Contains(PlayerState));

	ensureAlways(PlayerState->GetUniquePlayerID().IsValid());
	ensureAlways(WantedPlayers.Contains(PlayerState->GetUniquePlayerID()));
#endif

	UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

	// Apply the gameplay effect
	const FActiveGameplayEffectHandle ActiveWantedGameplayEffectHandle =
		AbilitySystemComponent->ApplyGameplayEffectToSelf(
			WantedGameplayEffectClass->GetDefaultObject<UGameplayEffect>(), 1,
			AbilitySystemComponent->MakeEffectContext());

#if DO_ENSURE
	ensureAlways(ActiveWantedGameplayEffectHandle.IsValid());
#endif

	// Remember the handle of the gameplay effect for this player to remove the gameplay effect later
	ActiveWantedGameplayEffectHandles.Add(AbilitySystemComponent, ActiveWantedGameplayEffectHandle);

	FOnActiveGameplayEffectRemoved_Info* OnWantedGameplayEffectRemoved =
		AbilitySystemComponent->OnGameplayEffectRemoved_InfoDelegate(ActiveWantedGameplayEffectHandle);

#if DO_CHECK
	check(OnWantedGameplayEffectRemoved);
#endif

	OnWantedGameplayEffectRemoved->AddUObject(this, &ThisClass::OnWantedGameplayEffectExternallyRemoved);
}

void UAlertScheduleEvent::RemoveWantedGameplayEffect(const AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Find an active gameplay effect handle for the given player
	const FActiveGameplayEffectHandle* ActiveWantedGameplayEffectHandle = ActiveWantedGameplayEffectHandles.Find(
		AbilitySystemComponent);

	// Don't do anything if there was no gameplay effect applied for this player by this event
	if (!ActiveWantedGameplayEffectHandle)
	{
		return;
	}

	FOnActiveGameplayEffectRemoved_Info* OnWantedGameplayEffectRemoved =
		AbilitySystemComponent->OnGameplayEffectRemoved_InfoDelegate(*ActiveWantedGameplayEffectHandle);

#if DO_CHECK
	check(OnWantedGameplayEffectRemoved);
#endif

	// Unsubscribe from the event that will be called when the gameplay effect is removed
	OnWantedGameplayEffectRemoved->RemoveAll(this);

	/**
	 * Remove the gameplay effect we applied before. NULL InstigatorAbilitySystemComponent means that the gameplay
	 * effect will be removed without checking if instigator is the same.
	 */
	AbilitySystemComponent->RemoveActiveGameplayEffect(*ActiveWantedGameplayEffectHandle, 1);

	// Forget about the handle for this player
	ActiveWantedGameplayEffectHandles.Remove(AbilitySystemComponent);
}

void UAlertScheduleEvent::OnWantedGameplayEffectExternallyRemoved(
	const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo)
{
	AActor* GameplayEffectInstigator = GameplayEffectRemovalInfo.EffectContext.GetInstigator();

#if DO_CHECK
	/**
	 * We had applied the gameplay effect to the player by calling "ApplyGameplayEffectToSelf" with "MakeEffectContext"
	 * function of the AbilitySystemComponent this player. And MakeEffectContext function sets the OwningActor as an
	 * instigator. And in our project OwningActor is an EscapeChroniclesPlayerState. So EffectContext of this gameplay
	 * effect must have an EscapeChroniclesPlayerState as an instigator, which is the same PlayerState that we applied
	 * the gameplay effect to.
	 */
	check(IsValid(GameplayEffectInstigator));
	check(GameplayEffectInstigator->IsA<AEscapeChroniclesPlayerState>());
#endif

	// PlayerState of the player that the gameplay effect was applied to
	AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(GameplayEffectInstigator);

	// Forget about the gameplay effect handle for this player
	ActiveWantedGameplayEffectHandles.Remove(PlayerState->GetAbilitySystemComponent());

	// Remove the player from the wanted players lists
	RemoveWantedPlayer(PlayerState);
}

FAlertScheduleEventSaveData UAlertScheduleEvent::GetAlertScheduleEventSaveData() const
{
	return FAlertScheduleEventSaveData(TSet(WantedPlayers));
}

void UAlertScheduleEvent::LoadAlertScheduleEventFromSaveData(const FAlertScheduleEventSaveData& SaveData)
{
#if DO_ENSURE
	ensureAlways(IsActive());
#endif

	SetWantedPlayers(SaveData.WantedPlayers);
}

void UAlertScheduleEvent::OnPlayerOrBotInitialized(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

#if DO_ENSURE
	ensureAlways(UniquePlayerID.IsValid());
#endif

	/**
	 * If the player has just joined the game but already was in the WantedPlayers array, then we need to call
	 * AddWantedPlayer for him to do everything needed for wanted players.
	 */
	if (WantedPlayers.Contains(UniquePlayerID) && !WantedPlayersInGame.Contains(PlayerState))
	{
		AddWantedPlayer(PlayerState);
	}
}

void UAlertScheduleEvent::OnPlayerOrBotLogout(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_ENSURE
	ensureAlways(IsValid(PlayerState));
#endif

	/**
	 * Remove the player from the list of wanted players that are currently in the game when the player leaves the game
	 * (or when the bot is destroyed).
	 */
	WantedPlayersInGame.Remove(PlayerState);
}

void UAlertScheduleEvent::OnEventEnded(const EScheduleEventEndReason EndReason)
{
	RemoveWantedGameplayEffectFromAllWantedPlayers();

	// Clear the wanted players
	WantedPlayers.Empty();
	WantedPlayersInGame.Empty();

	const UWorld* World = GetWorld();

	AEscapeChroniclesGameMode* GameMode = World->GetAuthGameMode<AEscapeChroniclesGameMode>();

	// Unsubscribe from GameMode events
	if (IsValid(GameMode))
	{
		GameMode->OnPlayerOrBotInitialized.Remove(OnPlayerOrBotInitializedDelegateHandle);
		GameMode->OnPlayerOrBotLogout.Remove(OnPlayerOrBotLogoutDelegateHandle);
	}

	// Clear all holes in the destructible meshes if the event ended normally
	if (EndReason == EScheduleEventEndReason::Normal)
	{
		for (TActorIterator<ADynamicMeshActor> It(World); It; ++It)
		{
#if DO_CHECK
			check(IsValid(*It));
#endif

			UDestructibleComponent* DestructibleComponent = It->FindComponentByClass<UDestructibleComponent>();

			if (IsValid(DestructibleComponent))
			{
				DestructibleComponent->ClearAllHoles();
			}
		}
	}

	Super::OnEventEnded(EndReason);
}

void UAlertScheduleEvent::OnEventPaused()
{
	RemoveWantedGameplayEffectFromAllWantedPlayers();

	Super::OnEventPaused();
}

void UAlertScheduleEvent::OnEventResumed()
{
	Super::OnEventResumed();

	ApplyWantedGameplayEffectToAllWantedPlayers();
}