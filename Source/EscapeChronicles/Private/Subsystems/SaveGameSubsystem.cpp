// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SaveGameSubsystem.h"

#include "EngineUtils.h"
#include "Common/Structs/ActorSaveData.h"
#include "Interfaces/Saveable.h"
#include "Kismet/GameplayStatics.h"
#include "Objects/EscapeChroniclesSaveGame.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void USaveGameSubsystem::SaveGame(const FString& SlotName)
{
	UEscapeChroniclesSaveGame* SaveGameObject = CastChecked<UEscapeChroniclesSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UEscapeChroniclesSaveGame::StaticClass()));

#if DO_CHECK
	check(IsValid(SaveGameObject));
#endif

	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;

#if DO_CHECK
		check(IsValid(Actor));
#endif

		// Skip actors that don't implement an interface or were dynamically spawned
		if (!Actor->Implements<USaveable>() || !Actor->HasAnyFlags(RF_WasLoaded))
		{
			continue;
		}

		FActorSaveData ActorSaveData;

		// Save actor's transform and all properties marked with "SaveGame"
		ActorSaveData.ActorSaveData.Transform = Actor->GetTransform();
		SaveObjectSaveGameFields(Actor, ActorSaveData.ActorSaveData.ByteData);

		for (UActorComponent* Component : Actor->GetComponents())
		{
#if DO_CHECK
			check(IsValid(Component));
#endif

			// Skip components that don't implement an interface or were dynamically spawned
			if (!Component->Implements<USaveable>() || !Component->HasAnyFlags(RF_ClassDefaultObject))
			{
				continue;
			}

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
			ActorSaveData.ComponentsSaveData.Add(Component->GetFName(), ComponentSaveData);

			// Subscribe a component to the event to call OnGameSaved on it once the game is saved
			ISaveable* SaveableComponent = CastChecked<ISaveable>(Component);
			OnGameSaved_Internal.AddRaw(SaveableComponent, &ISaveable::OnGameSaved);
		}

		// Add actor's SaveData to the save game object
		SaveGameObject->AddStaticSavedActor(Actor->GetFName(), ActorSaveData);

		// Subscribe an actor to the event to call OnGameSaved on it once the game is saved
		ISaveable* SaveableActor = CastChecked<ISaveable>(Actor);
		OnGameSaved_Internal.AddRaw(SaveableActor, &ISaveable::OnGameSaved);
	}

	UGameplayStatics::AsyncSaveGameToSlot(SaveGameObject, SlotName, 0,
		FAsyncSaveGameToSlotDelegate::CreateUObject(this, &USaveGameSubsystem::OnAsyncSavingFinished));
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

void USaveGameSubsystem::OnAsyncSavingFinished(const FString& SlotName, int32 UserIndex, bool bSuccess) const
{
	if (!bSuccess)
	{
		OnFailedToSaveGame.Broadcast();

		return;
	}

	OnGameSaved_Internal.Broadcast();
	OnGameSaved.Broadcast();
}

void USaveGameSubsystem::TryLoadGame(const FString& SlotName) const
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		OnFailedToLoadGame.Broadcast();

		return;
	}

	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, 0, FAsyncLoadGameFromSlotDelegate::CreateUObject(this,
		&ThisClass::OnAsyncLoadingSaveGameObjectFinished));
}

void USaveGameSubsystem::OnAsyncLoadingSaveGameObjectFinished(const FString& SlotName, int32 UserIndex,
	USaveGame* SaveGameObject) const
{
#if DO_CHECK
	check(IsValid(SaveGameObject));
	check(SaveGameObject->IsA<UEscapeChroniclesSaveGame>());
#endif

	const UEscapeChroniclesSaveGame* CastedSaveGameObject = CastChecked<UEscapeChroniclesSaveGame>(SaveGameObject);

	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;

#if DO_CHECK
		check(IsValid(Actor));
#endif

		// Skip actors that don't implement an interface or were dynamically spawned
		if (!Actor->Implements<USaveable>() || !Actor->HasAnyFlags(RF_WasLoaded))
		{
			continue;
		}

		const FActorSaveData* ActorSaveData = CastedSaveGameObject->FindActorSaveData(Actor->GetFName());

		// Skip actors that don't have any save data
		if (!ActorSaveData)
		{
			continue;
		}

		// Load actor's transform and all properties marked with "SaveGame"
		Actor->SetActorTransform(ActorSaveData->ActorSaveData.Transform);
		LoadObjectSaveGameFields(Actor, ActorSaveData->ActorSaveData.ByteData);

		for (UActorComponent* Component : Actor->GetComponents())
		{
#if DO_CHECK
			check(IsValid(Component));
#endif

			// Skip components that don't implement an interface or were dynamically spawned
			if (!Component->Implements<USaveable>() || !Component->HasAnyFlags(RF_ClassDefaultObject))
			{
				continue;
			}

			const FSaveData* ComponentSaveData = ActorSaveData->ComponentsSaveData.Find(Component->GetFName());

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

			// Call OnGameLoaded on the component
			ISaveable* SaveableComponent = CastChecked<ISaveable>(Component);
			SaveableComponent->OnGameLoaded();
		}

		// Call OnGameLoaded on the actor
		ISaveable* SaveableActor = CastChecked<ISaveable>(Actor);
		SaveableActor->OnGameLoaded();
	}
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