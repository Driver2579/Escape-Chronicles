// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SaveGameSubsystem.h"

#include "EngineUtils.h"
#include "Common/Enums/CharacterRole.h"
#include "Common/Structs/SaveData/ActorSaveData.h"
#include "Common/Structs/SaveData/PlayerSaveData.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/Saveable.h"
#include "Kismet/GameplayStatics.h"
#include "Objects/EscapeChroniclesSaveGame.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

USaveGameSubsystem::USaveGameSubsystem()
{
	AllowedDynamicallySpawnedActorsClasses = {
		AGameModeBase::StaticClass(),
		AGameStateBase::StaticClass()
	};

	PlayerSpecificClasses = {
		APawn::StaticClass(),
		AEscapeChroniclesPlayerState::StaticClass(),
		AController::StaticClass()
	};
}

void USaveGameSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Automatically save the game every AutoSavePeriod seconds
	if (InWorld.GetNetMode() < NM_Client)
	{
		FTimerHandle TimerHandle;
		InWorld.GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::AutoSaveAsync, AutoSavePeriod,
			true);
	}
}

UEscapeChroniclesSaveGame* USaveGameSubsystem::GetOrCreateSaveGameObjectChecked()
{
	if (CurrentSaveGameObject)
	{
		return CurrentSaveGameObject;
	}

	CurrentSaveGameObject = CastChecked<UEscapeChroniclesSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UEscapeChroniclesSaveGame::StaticClass()),
		ECastCheckedType::NullAllowed);

#if DO_CHECK
	check(CurrentSaveGameObject);
#endif

	return CurrentSaveGameObject;
}

bool USaveGameSubsystem::IsAllowedDynamicallySpawnedActor(const AActor* Actor) const
{
#if DO_CHECK
	check(IsValid(Actor));
#endif

	for (UClass* AllowedClass : AllowedDynamicallySpawnedActorsClasses)
	{
		if (Actor->IsA(AllowedClass))
		{
			return true;
		}
	}

	return false;
}

bool USaveGameSubsystem::IsPlayerSpecificActor(const AActor* Actor) const
{
#if DO_CHECK
	check(IsValid(Actor));
#endif

	for (UClass* PlayerSpecificClass : PlayerSpecificClasses)
	{
		if (Actor->IsA(PlayerSpecificClass))
		{
			return true;
		}
	}

	return false;
}

void USaveGameSubsystem::SaveGame(FString SlotName, const bool bAsync)
{
	OnSaveGameCalled.Broadcast();

	bGameSavingInProgress = true;

	UEscapeChroniclesSaveGame* SaveGameObject = GetOrCreateSaveGameObjectChecked();

	/**
	 * Clear the save game object to avoid saving old data, except we don't want to clear the data for the players
	 * because some players that were previously saved may be not in the game right now. We want to keep their data.
	 */
	SaveGameObject->ClearSavedActors();
	SaveGameObject->ClearBotsSaveData();

	// Clear the delegate to avoid duplicated binding and calling OnGameSaved on actors that can't be saved anymore
	OnGameSaved_Internal.Clear();

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);

	// Remember the level we're saving this SaveGameObject for
	SaveGameObject->SetLevelName(CurrentLevelName);

	// Add the level name to the slot name to know which slot for which level we need to load the game from when loading
	SlotName += SlotNameSeparator + CurrentLevelName;

	const UWorld* World = GetWorld();

	// Save world subsystems
	World->ForEachSubsystem<UWorldSubsystem>([this, SaveGameObject](UWorldSubsystem* WorldSubsystem)
	{
		ISaveable* SaveableSubsystem = Cast<ISaveable>(WorldSubsystem);

		/**
		 * Go to the next world subsystem if this one doesn't implement the Saveable interface or currently can't be
		 * saved.
		 */
		if (!SaveableSubsystem || !SaveableSubsystem->CanBeSavedOrLoaded())
		{
			return;
		}

		FSaveData WorldSubsystemSaveData;

		// Notify the subsystem it's about to be saved
		SaveableSubsystem->OnPreSaveObject();

		// Save the subsystem to the SaveData
		SaveObjectSaveGameFields(WorldSubsystem, WorldSubsystemSaveData.ByteData);

		// Add world subsystem's SaveData to the SaveGameObject
		SaveGameObject->AddWorldSubsystemSaveData(WorldSubsystem->GetClass(), WorldSubsystemSaveData);

		// Subscribe a subsystem to the event to call OnGameSaved on it once the game is saved
		OnGameSaved_Internal.AddRaw(SaveableSubsystem, &ISaveable::OnGameSaved);
	});

	// Save actors
	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;

#if DO_CHECK
		check(IsValid(Actor));
#endif

		const ISaveable* SaveableActor = Cast<ISaveable>(Actor);

		// Skip actors that don't implement an interface and actors that currently can't be saved
		if (!SaveableActor || !SaveableActor->CanBeSavedOrLoaded())
		{
			continue;
		}

		AEscapeChroniclesPlayerState* PlayerState = Cast<AEscapeChroniclesPlayerState>(Actor);

		// Save the player state separately with the player-specific actors
		if (PlayerState)
		{
			SavePlayerOrBotChecked(SaveGameObject, PlayerState);

			continue;
		}

		// Check if an actor was dynamically spawned
		const bool bDynamicallySpawnedActor = !Actor->HasAnyFlags(RF_WasLoaded);

		/**
		 * Skip dynamically spawned actors if they are not in the allowed list and player-specific actors
		 * (player-specific actors are saved separately).
		 */
		const bool bCanSaveActor = (!bDynamicallySpawnedActor || IsAllowedDynamicallySpawnedActor(Actor)) &&
			!IsPlayerSpecificActor(Actor);

		if (!bCanSaveActor)
		{
			continue;
		}

		FActorSaveData ActorSaveData;
		SaveActorToSaveDataChecked(Actor, ActorSaveData);

		/**
		 * Add actor's SaveData to the save game object. If an actor wasn't dynamically spawned, then add it to static
		 * actors.
		 */
		if (!bDynamicallySpawnedActor)
		{
			SaveGameObject->AddStaticSavedActor(Actor->GetFName(), ActorSaveData);
		}
		// Otherwise, if an actor was dynamically spawned, then add it to dynamically spawned actors
		else
		{
			SaveGameObject->AddDynamicallySpawnedSavedActor(Actor->GetClass(), ActorSaveData);
		}
	}

	if (bAsync)
	{
		UGameplayStatics::AsyncSaveGameToSlot(SaveGameObject, SlotName, GetPlatformUserIndex(),
			FAsyncSaveGameToSlotDelegate::CreateUObject(this, &USaveGameSubsystem::OnSavingFinished));
	}
	else
	{
		const int32 PlatformUserIndex = GetPlatformUserIndex();

		const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, PlatformUserIndex);
		OnSavingFinished(SlotName, PlatformUserIndex, bSuccess);
	}
}

void USaveGameSubsystem::SavePlayerOrBotChecked(UEscapeChroniclesSaveGame* SaveGameObject,
	AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
#endif

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

	// Skip the player if it doesn't have a valid UniquePlayerID (which should never be the case)
	if (!ensureAlways(UniquePlayerID.IsValid()))
	{
		return;
	}

	APawn* Pawn = PlayerState->GetPawn();

	/**
	 * There might be a situation where the player is saved after his pawn was already unpossessed. This may happen due
	 * to the game close where the game gets saved on each player that exists (they exit one by one), so the game save
	 * may be called multiple times and already after this player was already saved.
	 */
	if (!IsValid(Pawn))
	{
		return;
	}

	FPlayerSaveData PlayerSaveData;

	// Save the PlayerState (it's already checked)
	FActorSaveData PlayerStateSaveData;
	SaveActorToSaveDataChecked(PlayerState, PlayerStateSaveData);
	PlayerSaveData.PlayerSpecificActorsSaveData.Add(APlayerState::StaticClass(), PlayerStateSaveData);

	// Save the Pawn if it's valid and implements Saveable interface
	if (Pawn->Implements<USaveable>())
	{
		FActorSaveData PawnSaveData;
		SaveActorToSaveDataChecked(Pawn, PawnSaveData);
		PlayerSaveData.PlayerSpecificActorsSaveData.Add(APawn::StaticClass(), PawnSaveData);
	}

	AController* Controller = PlayerState->GetOwningController();

	// Save the Controller if it's valid and implements Saveable interface
	if (ensureAlways(IsValid(Controller)) && Controller->Implements<USaveable>())
	{
		FActorSaveData ControllerSaveData;
		SaveActorToSaveDataChecked(Controller, ControllerSaveData);
		PlayerSaveData.PlayerSpecificActorsSaveData.Add(AController::StaticClass(), ControllerSaveData);
	}

	// If the NetID is valid here, then the player is for sure playing using the online-service
	if (!UniquePlayerID.NetID.IsEmpty())
	{
		/**
		 * If there are save data for offline players and this is a player that was previously saved as an offline one
		 * (he was offline, but now he is online), then more save data for offline players to save data for online
		 * players. Once moved, the next offline players we are probably going to save will simply ignore this check.
		 *
		 * Note: This means that if players once joined online, they can't continue their save game offline anymore and
		 * will start as new players next time they join offline. To continue their save, they have to join online again
		 * (this will not affect users that are logged in into Steam (or other platform) and playing in offline mode).
		 */
		if (SaveGameObject->FindOfflinePlayerSaveData(UniquePlayerID))
		{
			SaveGameObject->MoveOfflinePlayersSaveDataToOnlinePlayersSaveData();
		}

		// Save the player to the list for online players because he has a valid NetID
		SaveGameObject->OverrideOnlinePlayerSaveData(UniquePlayerID, PlayerSaveData);
	}
	// If it's not an online player, then check if it's a bot and save it if it is
	else if (PlayerState->IsABot())
	{
		UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

#if DO_CHECK
		check(IsValid(AbilitySystemComponent));
#endif

		const ECharacterRole CharacterRole = PlayerState->GetCharacterRole();

		// Save the bot depending on its role
		switch (CharacterRole)
		{
			case ECharacterRole::Prisoner:
				SaveGameObject->AddPrisonerBotSaveData(UniquePlayerID, PlayerSaveData);
				break;

			case ECharacterRole::Guard:
				SaveGameObject->AddGuardBotSaveData(UniquePlayerID, PlayerSaveData);
				break;

			case ECharacterRole::Medic:
				SaveGameObject->AddMedicBotSaveData(UniquePlayerID, PlayerSaveData);
				break;

			// This should never happen
			default:
#if DO_ENSURE
				ensureAlways(false);
#endif
				break;
		}
	}
	// If it's not an online player and not a bot, then it's an offline standalone player. Save his data.
	else
	{
		SaveGameObject->OverrideOfflineStandalonePlayerSaveData(UniquePlayerID, PlayerSaveData);
	}
}

void USaveGameSubsystem::SaveActorToSaveDataChecked(AActor* Actor, FActorSaveData& OutActorSaveData)
{
#if DO_CHECK
	check(IsValid(Actor));
	check(Actor->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(Actor->HasAuthority());
#endif

	ISaveable* SaveableActor = CastChecked<ISaveable>(Actor);

#if DO_ENSURE
	ensureAlways(SaveableActor->CanBeSavedOrLoaded());
#endif

	// Let the actor update its properties before saving it
	SaveableActor->OnPreSaveObject();

	// Save actor's transform and all properties marked with "SaveGame"
	OutActorSaveData.ActorSaveData.Transform = Actor->GetTransform();
	SaveObjectSaveGameFields(Actor, OutActorSaveData.ActorSaveData.ByteData);

	for (UActorComponent* Component : Actor->GetComponents())
	{
#if DO_CHECK
		check(IsValid(Component));
#endif

		// Skip components that don't implement an interface or were dynamically spawned
		if (!Component->Implements<USaveable>() || Component->HasAnyFlags(RF_ClassDefaultObject))
		{
			continue;
		}

		ISaveable* SaveableComponent = CastChecked<ISaveable>(Component);

		// Let the component update its properties before saving it
		SaveableComponent->OnPreSaveObject();

		FSaveData ComponentSaveData;

		const USceneComponent* SceneComponent = Cast<USceneComponent>(Component);

		// Save component's transform if it's a scene component
		if (SceneComponent)
		{
			ComponentSaveData.Transform = SceneComponent->GetRelativeTransform();
		}

		// Save component's properties marked with "SaveGame"
		SaveObjectSaveGameFields(Component, ComponentSaveData.ByteData);

		// Add component's SaveData to the actor's SaveData
		OutActorSaveData.ComponentsSaveData.Add(Component->GetFName(), ComponentSaveData);

		// Subscribe a component to the event to call OnGameSaved on it once the game is saved
		OnGameSaved_Internal.AddRaw(SaveableComponent, &ISaveable::OnGameSaved);
	}

	// Subscribe an actor to the event to call OnGameSaved on it once the game is saved
	OnGameSaved_Internal.AddRaw(SaveableActor, &ISaveable::OnGameSaved);
}

void USaveGameSubsystem::SaveObjectSaveGameFields(UObject* Object, TArray<uint8>& OutByteData)
{
	// Pass the array to be able to fill with data from an object
	FMemoryWriter MemoryWriter(OutByteData);

	// Create an archive to serialize the data from an object
	FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, true);

	// Serialize only properties marked with "SaveGame"
	Ar.ArIsSaveGame = true;

	// Finally, serialize the object's properties
	Object->Serialize(Ar);
}

void USaveGameSubsystem::OnSavingFinished(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	if (bSuccess)
	{
		OnGameSaved_Internal.Broadcast();
		OnGameSaved.Broadcast();
	}
	else
	{
		OnFailedToSaveGame.Broadcast();
	}

	bGameSavingInProgress = false;
}

void USaveGameSubsystem::LoadGameAndInitializeUniquePlayerIDs(FString SlotName, const bool bAsync)
{
	OnLoadGameCalled.Broadcast();

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);

	// Add the level name to the slot name to know which slot for which level we need to load the game from
	SlotName += SlotNameSeparator + CurrentLevelName;

	const int32 PlatformUserIndex = GetPlatformUserIndex();

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, PlatformUserIndex))
	{
		OnFailedToLoadGame.Broadcast();

		return;
	}

	bGameLoadingInProgress = true;

	if (bAsync)
	{
		UGameplayStatics::AsyncLoadGameFromSlot(SlotName, PlatformUserIndex,
			FAsyncLoadGameFromSlotDelegate::CreateUObject(this, &ThisClass::OnLoadingSaveGameObjectFinished));
	}
	else
	{
		USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, PlatformUserIndex);
		OnLoadingSaveGameObjectFinished(SlotName, PlatformUserIndex, SaveGameObject);
	}
}

void USaveGameSubsystem::OnLoadingSaveGameObjectFinished(const FString& SlotName, int32 UserIndex,
	USaveGame* SaveGameObject)
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(SaveGameObject->IsA<UEscapeChroniclesSaveGame>());
#endif

	// Override the save game object with a newly loaded one
	CurrentSaveGameObject = CastChecked<UEscapeChroniclesSaveGame>(SaveGameObject);

	// Make sure the level name that was saved in SaveGameObject matches the current level name
	const bool bLevelNamesMatch = ensureAlways(
		CurrentSaveGameObject->GetLevelName() == UGameplayStatics::GetCurrentLevelName(this));

	// Forbid loading the game if level names don't match
	if (!bLevelNamesMatch)
	{
		OnFailedToLoadGame.Broadcast();
		bGameLoadingInProgress = false;

		return;
	}

	const UWorld* World = GetWorld();

	// === Load world subsystems ===

	World->ForEachSubsystem<UWorldSubsystem>([this](UWorldSubsystem* WorldSubsystem)
	{
		ISaveable* SaveableSubsystem = Cast<ISaveable>(WorldSubsystem);

		// Try to load the world subsystem if it implements the Saveable interface and currently can be loaded
		if (SaveableSubsystem && SaveableSubsystem->CanBeSavedOrLoaded())
		{
			const FSaveData* WorldSubsystemSaveData = CurrentSaveGameObject->FindWorldSubsystemSaveData(
				WorldSubsystem->GetClass());

			if (WorldSubsystemSaveData)
			{
				// Notify the subsystem it's about to be loaded
				SaveableSubsystem->OnPreLoadObject();

				// Load the subsystem
				LoadObjectSaveGameFields(WorldSubsystem, WorldSubsystemSaveData->ByteData);

				// Notify the subsystem it was loaded
				SaveableSubsystem->OnPostLoadObject();
			}
		}
	});

	// === Load actors ===

	TArray<AActor*> StaticActors;
	TArray<AActor*> AllowedDynamicallySpawnedActors;
	TArray<AEscapeChroniclesPlayerState*> PlayerStates;

	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;

#if DO_CHECK
		check(IsValid(Actor));
#endif

		const ISaveable* SaveableActor = Cast<ISaveable>(Actor);

		// Skip actors that don't implement an interface and actors that currently can't be loaded
		if (!SaveableActor || !SaveableActor->CanBeSavedOrLoaded())
		{
			continue;
		}

		AEscapeChroniclesPlayerState* PlayerState = Cast<AEscapeChroniclesPlayerState>(Actor);

		// Try to add an Actor to PlayerStates if it's a PlayerState
		if (PlayerState)
		{
			PlayerStates.Add(PlayerState);

			continue;
		}

		// Skip player-specific actors because they are loaded separately
		if (IsPlayerSpecificActor(Actor))
		{
			continue;
		}

		// Try to add an Actor to AllowedDynamicallySpawnedActors if it's a dynamically spawned actor
		if (!Actor->HasAnyFlags(RF_WasLoaded))
		{
			if (IsAllowedDynamicallySpawnedActor(Actor))
			{
				AllowedDynamicallySpawnedActors.Add(Actor);
			}

			continue;
		}

		// If we passed all checks, then the actor is static
		StaticActors.Add(Actor);
	}

	// First, load StaticActors
	for (AActor* StaticActor : StaticActors)
	{
		const FActorSaveData* ActorSaveData = CurrentSaveGameObject->FindStaticActorSaveData(StaticActor->GetFName());

		// Load an actor if its save data is valid
		if (ActorSaveData)
		{
			LoadActorFromSaveDataChecked(StaticActor, *ActorSaveData);
		}
	}

	// Then load AllowedDynamicallySpawnedActors
	for (AActor* AllowedDynamicallySpawnedActor : AllowedDynamicallySpawnedActors)
	{
		const FActorSaveData* ActorSaveData = CurrentSaveGameObject->FindDynamicallySpawnedActorSaveData(
			AllowedDynamicallySpawnedActor->GetClass());

		// Load an actor if its save data is valid
		if (ActorSaveData)
		{
			LoadActorFromSaveDataChecked(AllowedDynamicallySpawnedActor, *ActorSaveData);
		}
	}

	// Then load players
	for (AEscapeChroniclesPlayerState* PlayerState : PlayerStates)
	{
		if (!PlayerState->IsABot())
		{
			LoadPlayerOrGenerateUniquePlayerIdChecked(CurrentSaveGameObject, PlayerState);
		}
	}

	// Then load bots
	for (AEscapeChroniclesPlayerState* PlayerState : PlayerStates)
	{
		if (PlayerState->IsABot())
		{
			LoadBotOrGenerateUniquePlayerIdChecked(CurrentSaveGameObject, PlayerState);
		}
	}

	OnGameLoaded.Broadcast();

	bGameLoadingInProgress = false;
}

bool USaveGameSubsystem::LoadPlayerOrGenerateUniquePlayerIdChecked(const UEscapeChroniclesSaveGame* SaveGameObject,
	AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
	ensureAlways(!PlayerState->IsABot());
#endif

	const FPlayerSaveData* PlayerSaveData = LoadOrGenerateUniquePlayerIdAndLoadSaveDataForPlayer(SaveGameObject, PlayerState);

	// Don't load the player if he doesn't have anything to load
	if (!PlayerSaveData)
	{
		return false;
	}

	LoadPlayerSpecificActors(*PlayerSaveData, PlayerState);

	return true;
}

const FPlayerSaveData* USaveGameSubsystem::LoadOrGenerateUniquePlayerIdAndLoadSaveDataForPlayer(
	const UEscapeChroniclesSaveGame* SaveGameObject, AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
	check(IsValid(PlayerState->GetPlayerController()));
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
	ensureAlways(!PlayerState->IsABot());
#endif

	const FPlayerSaveData* PlayerSaveData;

	/**
	 * Generate the UniquePlayerID if it's not valid already. The PlayerID from here can be overriden by the one
	 * from the save, but NetID and LocalPlayerID won't be changed.
	 */
	PlayerState->GenerateUniquePlayerIdIfInvalid();

	FUniquePlayerID UniquePlayerID = PlayerState->GetUniquePlayerID();

	// Load the save data for the player as an online player if the player is online
	if (PlayerState->IsOnlinePlayer())
	{
		/**
		 * Find the save data for the given NetID and get the PlayerID from the save data. If it's not found, just keep
		 * the UniquePlayerID we just generated for now.
		 */
		PlayerSaveData = SaveGameObject->FindOnlinePlayerSaveDataAndUpdatePlayerID(UniquePlayerID);

		/**
		 * If we failed to find the save data for the player and the player is locally controlled (this code is executed
		 * on server), then it's probably a hosting player that used to play offline before (or another player that uses
		 * the same save). But maybe we don't have a saved data for the offline player at all.
		 */
		const bool bIsHostThatMaybeWasSavedOffline = !PlayerSaveData &&
			PlayerState->GetPlayerController()->IsLocalController();

		/**
		 * Try to find the data for this player in the offline players list if it's a host that maybe was saved offline
		 * in previous sessions.
		 */
		if (bIsHostThatMaybeWasSavedOffline)
		{
			PlayerSaveData = LoadOfflinePlayerSaveDataAndPlayerID(SaveGameObject, UniquePlayerID);
		}
	}
	// Otherwise, load the save data for the player as an offline player because he's playing offline
	else
	{
		PlayerSaveData = LoadOfflinePlayerSaveDataAndPlayerID(SaveGameObject, UniquePlayerID);
	}

	// Set the UniquePlayerID once we loaded or generated it
	PlayerState->SetUniquePlayerID(UniquePlayerID);

	return PlayerSaveData;
}

const FPlayerSaveData* USaveGameSubsystem::LoadOfflinePlayerSaveDataAndPlayerID(
	const UEscapeChroniclesSaveGame* SaveGameObject, FUniquePlayerID& InOutUniquePlayerID)
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(InOutUniquePlayerID.IsValid());
#endif

	const FPlayerSaveData* OfflinePlayerSaveData;
	uint64 PlayerID;

	const bool bWasSaved = SaveGameObject->FindOfflinePlayerSaveDataAndPlayerIdByLocalPlayerID(
		InOutUniquePlayerID.LocalPlayerID, OfflinePlayerSaveData, PlayerID);

	/**
	 * If we have a saved data for this offline player, then we can use it for this player. Otherwise, we just keep the
	 * PlayerID we generated above.
	 */
	if (bWasSaved)
	{
		InOutUniquePlayerID.PlayerID = PlayerID;

		return OfflinePlayerSaveData;
	}

	return nullptr;
}

bool USaveGameSubsystem::LoadBotOrGenerateUniquePlayerIdChecked(const UEscapeChroniclesSaveGame* SaveGameObject,
	AEscapeChroniclesPlayerState* PlayerState) const
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
	ensureAlways(PlayerState->IsABot());
#endif

	const FPlayerSaveData* BotSaveData = LoadOrGenerateUniquePlayerIdAndLoadSaveDataForBot(SaveGameObject, PlayerState);

	// Don't load the bot if it doesn't have anything to load
	if (!BotSaveData)
	{
		return false;
	}

	LoadPlayerSpecificActors(*BotSaveData, PlayerState);

	return true;
}

const FPlayerSaveData* USaveGameSubsystem::LoadOrGenerateUniquePlayerIdAndLoadSaveDataForBot(
	const UEscapeChroniclesSaveGame* SaveGameObject, AEscapeChroniclesPlayerState* PlayerState) const
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
	ensureAlways(PlayerState->IsABot());
#endif

	const ECharacterRole CharacterRole = PlayerState->GetCharacterRole();

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

	/**
	 * Try to find the save data for a bot if it already has a UniquePlayerID. If failed to find, then return null as
	 * this bot already has a UniquePlayerID and don't have any save data load.
	 */
	if (UniquePlayerID.IsValid())
	{
		// Find the save data depending on the bot's role
		switch (CharacterRole)
		{
			case ECharacterRole::Prisoner:
				return SaveGameObject->FindPrisonerBotSaveData(UniquePlayerID);

			case ECharacterRole::Guard:
				return SaveGameObject->FindGuardBotSaveData(UniquePlayerID);

			case ECharacterRole::Medic:
				return SaveGameObject->FindMedicBotSaveData(UniquePlayerID);

			// This should never happen
			default:
#if DO_ENSURE
				ensureAlways(false);
#endif

				return nullptr;
		}
	}

	// Contains the save data of all bots that have the same role as the one we are trying to load
	const TMap<FUniquePlayerID, FPlayerSaveData>* BotsSaveDataOfBotType;

	// Get the save data depending on the bot's role
	switch (CharacterRole)
	{
		case ECharacterRole::Prisoner:
			BotsSaveDataOfBotType = &SaveGameObject->GetPrisonerBotsSaveData();
			break;

		case ECharacterRole::Guard:
			BotsSaveDataOfBotType = &SaveGameObject->GetGuardBotsSaveData();
			break;

		case ECharacterRole::Medic:
			BotsSaveDataOfBotType = &SaveGameObject->GetMedicBotsSaveData();
			break;

		// This should never happen
		default:
#if DO_ENSURE
			ensureAlways(false);
#endif

			BotsSaveDataOfBotType = nullptr;
	}

	if (BotsSaveDataOfBotType)
	{
		for (const TPair<FUniquePlayerID, FPlayerSaveData>& BotSaveDataPair : *BotsSaveDataOfBotType)
		{
			// Go to the next pair if UniquePlayerID from this pair was already taken by another bot
			if (!RegisteredBotsUniquePlayerIDs.Contains(BotSaveDataPair.Key))
			{
				continue;
			}

			// Initialize the UniquePlayerID of the bot with the first untaken UniquePlayerID from the saved data
			PlayerState->SetUniquePlayerID(BotSaveDataPair.Key);

#if DO_ENSURE
			/**
			 * Make sure the bot has called the RegisterBotUniquePlayerID function after we called the SetUniquePlayerID
			 * function for this bot.
			 */
			EnsureBotUniquePlayerIdIsRegistered(PlayerState->GetUniquePlayerID());
#endif

			// Return the found save data for the bot to load
			return &BotSaveDataPair.Value;
		}
	}

	/**
	 * If we get here, then we failed to find the UniquePlayerID and the save data to load for the bot. Generate the new
	 * UniquePlayerID for the bot in this case (it will call the RegisterBotUniquePlayerID function automatically).
	 */
	PlayerState->GenerateUniquePlayerIdIfInvalid();

#if DO_ENSURE
	/**
	 * Make sure the bot has called the RegisterBotUniquePlayerID function after we called the
	 * GenerateUniquePlayerIdIfInvalid function for this bot.
	 */
	EnsureBotUniquePlayerIdIsRegistered(PlayerState->GetUniquePlayerID());
#endif

	// We didn't find any save data, so return null
	return nullptr;
}

void USaveGameSubsystem::LoadPlayerSpecificActors(const FPlayerSaveData& PlayerOrBotSaveData,
	AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
#endif

	const FActorSaveData* PlayerStateSaveData = PlayerOrBotSaveData.PlayerSpecificActorsSaveData.Find(
		APlayerState::StaticClass());

	// Load the PlayerState if its save data is valid
	if (PlayerStateSaveData)
	{
		LoadActorFromSaveDataChecked(PlayerState, *PlayerStateSaveData);
	}

	APawn* PlayerPawn = PlayerState->GetPawn();

	// Try to load the Pawn if it's valid and implements Saveable interface
	if (ensureAlways(IsValid(PlayerPawn)) && PlayerPawn->Implements<USaveable>())
	{
		const FActorSaveData* PawnSaveData = PlayerOrBotSaveData.PlayerSpecificActorsSaveData.Find(
			APawn::StaticClass());

		// Load the Pawn if its save data is valid
		if (PawnSaveData)
		{
			LoadActorFromSaveDataChecked(PlayerPawn, *PawnSaveData);
		}
	}

	AController* Controller = PlayerState->GetOwningController();

	// Try to load the Controller if it's valid and implements Saveable interface
	if (ensureAlways(IsValid(Controller)) && Controller->Implements<USaveable>())
	{
		const FActorSaveData* ControllerSaveData = PlayerOrBotSaveData.PlayerSpecificActorsSaveData.Find(
			AController::StaticClass());

		// Load the Controller if its save data is valid
		if (ControllerSaveData)
		{
			LoadActorFromSaveDataChecked(Controller, *ControllerSaveData);
		}
	}
}

void USaveGameSubsystem::LoadActorFromSaveDataChecked(AActor* Actor, const FActorSaveData& ActorSaveData)
{
#if DO_CHECK
	check(IsValid(Actor));
	check(Actor->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(Actor->HasAuthority());
#endif

	ISaveable* SaveableActor = CastChecked<ISaveable>(Actor);

#if DO_ENSURE
	ensureAlways(SaveableActor->CanBeSavedOrLoaded());
#endif

	// Notify the actor it's about to be loaded
	SaveableActor->OnPreLoadObject();

	// Load actor's transform if its RootComponent is Movable
	if (Actor->GetRootComponent()->Mobility == EComponentMobility::Movable)
	{
		Actor->SetActorTransform(ActorSaveData.ActorSaveData.Transform);
	}

	// Load all actor's properties marked with "SaveGame"
	LoadObjectSaveGameFields(Actor, ActorSaveData.ActorSaveData.ByteData);

	for (UActorComponent* Component : Actor->GetComponents())
	{
#if DO_CHECK
		check(IsValid(Component));
#endif

		// Skip components that don't implement an interface or were dynamically spawned
		if (!Component->Implements<USaveable>() || Component->HasAnyFlags(RF_ClassDefaultObject))
		{
			continue;
		}

		ISaveable* SaveableComponent = CastChecked<ISaveable>(Component);

		// Notify the component it's about to be loaded
		SaveableComponent->OnPreLoadObject();

		const FSaveData* ComponentSaveData = ActorSaveData.ComponentsSaveData.Find(Component->GetFName());

		// Skip components that don't have any save data
		if (!ComponentSaveData)
		{
			continue;
		}

		USceneComponent* SceneComponent = Cast<USceneComponent>(Component);

		// Load component's transform if it's a movable scene component
		if (SceneComponent && SceneComponent->Mobility == EComponentMobility::Movable)
		{
			SceneComponent->SetRelativeTransform(ComponentSaveData->Transform);
		}

		// Load component's properties marked with "SaveGame"
		LoadObjectSaveGameFields(Component, ComponentSaveData->ByteData);

		// Notify the component it's loaded
		SaveableComponent->OnPostLoadObject();
	}

	// Notify the actor it's loaded
	SaveableActor->OnPostLoadObject();
}

void USaveGameSubsystem::LoadObjectSaveGameFields(UObject* Object, const TArray<uint8>& InByteData)
{
	// Pass the array to read the data for an object from
	FMemoryReader MemoryReader(InByteData);

	// Serialize the passed data to an archive
	FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);

	// Serialize only properties marked with "SaveGame"
	Ar.ArIsSaveGame = true;

	// Finally, serialize the object's properties from the archive
	Object->Serialize(Ar);
}

bool USaveGameSubsystem::LoadPlayerOrGenerateUniquePlayerID(AEscapeChroniclesPlayerState* PlayerState) const
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	if (CurrentSaveGameObject)
	{
		return LoadPlayerOrGenerateUniquePlayerIdChecked(CurrentSaveGameObject, PlayerState);
	}

	// Just Generate a UniquePlayerID for the player if we don't have a save game object
	PlayerState->GenerateUniquePlayerIdIfInvalid();

	return false;
}

bool USaveGameSubsystem::LoadBotOrGenerateUniquePlayerID(AEscapeChroniclesPlayerState* PlayerState) const
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	if (CurrentSaveGameObject)
	{
		return LoadBotOrGenerateUniquePlayerIdChecked(CurrentSaveGameObject, PlayerState);
	}

	// Just Generate a UniquePlayerID for the bot if we don't have a save game object
	PlayerState->GenerateUniquePlayerIdIfInvalid();

#if DO_ENSURE
	/**
	 * Make sure the bot has called the RegisterBotUniquePlayerID function after we called the
	 * GenerateUniquePlayerIdIfInvalid function for this bot.
	 */
	EnsureBotUniquePlayerIdIsRegistered(PlayerState->GetUniquePlayerID());
#endif

	return false;
}

int32 USaveGameSubsystem::GetPlatformUserIndex() const
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!IsValid(LocalPlayer))
	{
		return 0;
	}

	return LocalPlayer->GetPlatformUserIndex();
}