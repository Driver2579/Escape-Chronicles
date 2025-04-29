// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SaveGameSubsystem.h"

#include "EngineUtils.h"
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
		APlayerController::StaticClass()
	};
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

void USaveGameSubsystem::SaveGame(const FString& SlotName, const bool bAsync)
{
	UEscapeChroniclesSaveGame* SaveGameObject = GetOrCreateSaveGameObjectChecked();

	// Clear the save game object to avoid saving old data
	SaveGameObject->ClearSavedActors();

	// Clear the delegate to avoid duplicated binding and calling OnGameSaved on actors that can't be saved anymore
	OnGameSaved_Internal.Clear();

	for (FActorIterator It(GetWorld()); It; ++It)
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
			SavePlayerToSaveGameObjectChecked(SaveGameObject, PlayerState);

			continue;
		}

		// Skip dynamically spawned actors and player-specific actors (player-specific actors are saved separately)
		if (!Actor->HasAnyFlags(RF_WasLoaded) || IsPlayerSpecificActor(Actor))
		{
			continue;
		}

		FActorSaveData ActorSaveData;
		SaveActorToSaveDataChecked(Actor, ActorSaveData);

		// Add actor's SaveData to the save game object
		SaveGameObject->AddStaticSavedActor(Actor->GetFName(), ActorSaveData);
	}

	if (bAsync)
	{
		UGameplayStatics::AsyncSaveGameToSlot(SaveGameObject, SlotName, 0,
			FAsyncSaveGameToSlotDelegate::CreateUObject(this, &USaveGameSubsystem::OnSavingFinished));
	}
	else
	{
		const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);
		OnSavingFinished(SlotName, 0, bSuccess);
	}
}

void USaveGameSubsystem::SavePlayerToSaveGameObjectChecked(UEscapeChroniclesSaveGame* SaveGameObject,
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

	const FUniquePlayerID& PlayerID = PlayerState->GetUniquePlayerID();

	// Skip the player if it doesn't have a valid net ID
	if (!ensureAlways(PlayerID.IsValid()))
	{
		return;
	}

	FPlayerSaveData PlayerSaveData;

	// Save the PlayerState (it's already checked)
	FActorSaveData PlayerStateSaveData;
	SaveActorToSaveDataChecked(PlayerState, PlayerStateSaveData);
	PlayerSaveData.PlayerSpecificActorsSaveData.Add(APlayerState::StaticClass(), PlayerStateSaveData);

	APawn* Pawn = PlayerState->GetPawn();

	// Save the Pawn if it's valid and implements Saveable interface
	if (ensureAlways(IsValid(Pawn)) && Pawn->Implements<USaveable>())
	{
		FActorSaveData PawnSaveData;
		SaveActorToSaveDataChecked(Pawn, PawnSaveData);
		PlayerSaveData.PlayerSpecificActorsSaveData.Add(APawn::StaticClass(), PawnSaveData);
	}

	APlayerController* PlayerController = PlayerState->GetPlayerController();

	// Save the PlayerController if it's valid and implements Saveable interface
	if (ensureAlways(IsValid(PlayerController)) && PlayerController->Implements<USaveable>())
	{
		FActorSaveData PlayerControllerSaveData;
		SaveActorToSaveDataChecked(PlayerController, PlayerControllerSaveData);
		PlayerSaveData.PlayerSpecificActorsSaveData.Add(APlayerController::StaticClass(),
			PlayerControllerSaveData);
	}

	SaveGameObject->OverridePlayerSaveData(PlayerID, PlayerSaveData);
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

void USaveGameSubsystem::OnSavingFinished(const FString& SlotName, int32 UserIndex, bool bSuccess) const
{
	if (!bSuccess)
	{
		OnFailedToSaveGame.Broadcast();

		return;
	}

	OnGameSaved_Internal.Broadcast();
	OnGameSaved.Broadcast();
}

void USaveGameSubsystem::TryLoadGame(const FString& SlotName, const bool bAsync)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		OnFailedToLoadGame.Broadcast();

		return;
	}

	if (bAsync)
	{
		UGameplayStatics::AsyncLoadGameFromSlot(SlotName, 0,
			FAsyncLoadGameFromSlotDelegate::CreateUObject(this, &ThisClass::OnLoadingSaveGameObjectFinished));
	}
	else
	{
		USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
		OnLoadingSaveGameObjectFinished(SlotName, 0, SaveGameObject);
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

	TArray<AActor*> StaticActors;
	TArray<AActor*> AllowedDynamicallySpawnedActors;
	TArray<AEscapeChroniclesPlayerState*> PlayerStates;

	for (FActorIterator It(GetWorld()); It; ++It)
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
		if (Actor->HasAnyFlags(RF_WasLoaded))
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

	// First load StaticActors
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
		LoadPlayerFromSaveGameObjectChecked(CurrentSaveGameObject, PlayerState);
	}
}

bool USaveGameSubsystem::LoadPlayerFromSaveGameObjectChecked(const UEscapeChroniclesSaveGame* SaveGameObject,
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

	/**
	 * TODO:
	 * Это не безопасно. Может произойти ситуация, что хост будет иметь не первый сгенерированный ID. Например, если он
	 * перекинет сейв другому игроку, на котором до этого хост уже подключался онлайн. Тогда новый игрок получит сейв,
	 * на котором уже есть минимум один ID, зарезервированный под NetID, и у нового клиента сгенерируется уже не нулевой
	 * ID. Нам надо отдельно сохранять ID хоста, если у него не привязан NetID. То есть, подключаемся -> пытаемся
	 * загрузить сейв по NetID -> если не получается, то проверяем, является ли игрок хостом -> если нет, то просто
	 * генерируем новый ID и останавливаем цепочку, а если да, то проверяем есть ли сохраненный FUniquePlayerID для
	 * хоста без NetID (или можно сохранять просто uint64 PlayerID) -> если есть, то присваиваем PlayerID от него к
	 * подключенному игроку, а если нет, то просто генерируем новый ID и останавливаем цепочку -> если у подключенного
	 * игрока есть NetID, то удаляем тот PlayerID, который был сохранен (тогда выходит, что у нас больше нет
	 * сохраненного офлайн хоста, ибо игрок, подключившийся онлайн, его перехватил). При сохранении, соответственно,
	 * надо будет тоже проверять является ли игрок хостом и нет ли у него NetID. Если хост без NetID, то сохраняем как
	 * новый ID хоста.
	 */
	// Generate a UniquePlayerID for the PlayerState if it doesn't already have one
	PlayerState->GenerateUniquePlayerIdIfInvalid();

	FUniquePlayerID& PlayerID = PlayerState->GetUniquePlayerID();

	const FPlayerSaveData* PlayerSaveData = SaveGameObject->FindPlayerSaveDataAndUpdatePlayerID(PlayerID);

	// Don't load the player if it doesn't have anything to load. Usually, we should always have it if the ID is valid.
	if (!PlayerSaveData)
	{
		return false;
	}

	const FActorSaveData* PlayerStateSaveData = PlayerSaveData->PlayerSpecificActorsSaveData.Find(
		APlayerState::StaticClass());

	// Load the PlayerState if its save data is valid
	if (ensureAlways(PlayerStateSaveData))
	{
		LoadActorFromSaveDataChecked(PlayerState, *PlayerStateSaveData);
	}

	APawn* PlayerPawn = PlayerState->GetPawn();

	// Try to load the Pawn if it's valid and implements Saveable interface
	if (ensureAlways(IsValid(PlayerPawn)) && PlayerPawn->Implements<USaveable>())
	{
		const FActorSaveData* PawnSaveData = PlayerSaveData->PlayerSpecificActorsSaveData.Find(
			APawn::StaticClass());

		// Load the Pawn if its save data is valid
		if (PawnSaveData)
		{
			LoadActorFromSaveDataChecked(PlayerPawn, *PawnSaveData);
		}
	}

	APlayerController* PlayerController = PlayerState->GetPlayerController();

	// Try to load the PlayerController if it's valid and implements Saveable interface
	if (ensureAlways(IsValid(PlayerController)) && PlayerController->Implements<USaveable>())
	{
		const FActorSaveData* PlayerControllerSaveData = PlayerSaveData->PlayerSpecificActorsSaveData.Find(
			APlayerController::StaticClass());

		// Load the PlayerController if its save data is valid
		if (PlayerControllerSaveData)
		{
			LoadActorFromSaveDataChecked(PlayerController, *PlayerControllerSaveData);
		}
	}

	return true;
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

	// Load actor's transform and all properties marked with "SaveGame"
	Actor->SetActorTransform(ActorSaveData.ActorSaveData.Transform);
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

		// Load component's transform if it's a scene component
		if (SceneComponent)
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

bool USaveGameSubsystem::TryLoadPlayerFromCurrentSaveGameObject(AEscapeChroniclesPlayerState* PlayerState) const
{
#if DO_CHECK
	check(PlayerState);
#endif

	if (ensureAlways(CurrentSaveGameObject))
	{
		return LoadPlayerFromSaveGameObjectChecked(CurrentSaveGameObject, PlayerState);
	}

	return false;
}