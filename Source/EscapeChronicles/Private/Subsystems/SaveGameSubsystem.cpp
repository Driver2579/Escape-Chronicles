// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SaveGameSubsystem.h"

#include "EngineUtils.h"
#include "Common/Structs/SaveData/ActorSaveData.h"
#include "Common/Structs/SaveData/PlayerSaveData.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/Saveable.h"
#include "Kismet/GameplayStatics.h"
#include "Objects/EscapeChroniclesSaveGame.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

USaveGameSubsystem::USaveGameSubsystem()
{
	PlayerSpecificClasses = {
		APawn::StaticClass(),
		APlayerState::StaticClass(),
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
	SaveGameObject->ClearStaticSavedActors();

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

		APlayerState* PlayerState = Cast<APlayerState>(Actor);

		// Save the player state separately with the player-specific actors
		if (PlayerState)
		{
			SavePlayerToSaveGameObjectChecked(SaveGameObject, PlayerState);

			continue;
		}

		// Skip dynamically spawned actors and player-specific actors (player-specific actors are saved separately)
		if (!Actor->HasAnyFlags(RF_WasLoaded) && !IsPlayerSpecificActor(Actor))
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
	APlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
#endif

	const FUniqueNetIdRepl& PlayerNetID = PlayerState->GetUniqueId();

	// Skip the player if it doesn't have a valid net ID
	if (!ensureAlways(PlayerNetID.IsValid()))
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

	SaveGameObject->OverridePlayerSaveData(PlayerNetID, PlayerSaveData);
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

		APlayerState* PlayerState = Cast<APlayerState>(Actor);

		// Load the player state separately with the player-specific actors
		if (PlayerState)
		{
			LoadPlayerFromSaveGameObjectChecked(CurrentSaveGameObject, PlayerState);

			continue;
		}

		// Skip dynamically spawned actors and player-specific actors (player-specific actors are loaded separately)
		if (!Actor->HasAnyFlags(RF_WasLoaded) && !IsPlayerSpecificActor(Actor))
		{
			continue;
		}

		const FActorSaveData* ActorSaveData = CurrentSaveGameObject->FindStaticActorSaveData(Actor->GetFName());

		// Load an actor if its save data is valid
		if (ActorSaveData)
		{
			LoadActorFromSaveDataChecked(Actor, *ActorSaveData);
		}
	}
}

void USaveGameSubsystem::LoadPlayerFromSaveGameObjectChecked(const UEscapeChroniclesSaveGame* SaveGameObject,
	APlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(IsValid(PlayerState));
	check(PlayerState->Implements<USaveable>());
#endif

#if DO_ENSURE
	ensureAlways(PlayerState->HasAuthority());
#endif

	const FUniqueNetIdRepl& PlayerNetID = PlayerState->GetUniqueId();

	// Skip the player if it doesn't have a valid net ID
	if (!ensureAlways(PlayerNetID.IsValid()))
	{
		return;
	}

	const FPlayerSaveData* PlayerSaveData = SaveGameObject->FindPlayerSaveData(PlayerNetID);

	// Don't load the player if it doesn't have anything to load
	if (!PlayerSaveData)
	{
		return;
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

void USaveGameSubsystem::TryLoadPlayerFromCurrentSaveGameObject(APlayerState* PlayerState) const
{
#if DO_CHECK
	check(PlayerState);
#endif

	if (ensureAlways(CurrentSaveGameObject))
	{
		LoadPlayerFromSaveGameObjectChecked(CurrentSaveGameObject, PlayerState);
	}
}