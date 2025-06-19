// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Interfaces/Saveable.h"
#include "SaveInventoryPickupItemsSubsystem.generated.h"

class AEscapeChroniclesInventoryPickupItem;

struct FInventoryTypedSlotSaveData;
struct FStreamableHandle;
struct FPickupItemSaveData;

/**
 * A subsystem that handles saving and loading the inventory pickup items in the world. It handles their spawning and
 * destroying when the game is loaded, and saves and loads theis item instances.
 */
UCLASS()
class ESCAPECHRONICLES_API USaveInventoryPickupItemsSubsystem : public UWorldSubsystem, public ISaveable
{
	GENERATED_BODY()

public:
	/**
	 * This must be called from any AEscapeChroniclesInventoryPickupItem-derived class when the item is spawned in the
	 * world (in BeginPlay).
	 */
	void RegisterPickupItem(AEscapeChroniclesInventoryPickupItem* PickupItem);

	virtual void OnPreSaveObject() override;
	virtual void OnPostLoadObject() override;

protected:
	UFUNCTION()
	virtual void OnPickupItemDestroyed(AActor* DestroyedActor);

private:
	// All pickup items that are currently in the world
	TSet<TObjectPtr<AEscapeChroniclesInventoryPickupItem>> PickupItems;

	// All item instances and their save data that were saved
	UPROPERTY(Transient, SaveGame)
	TArray<FPickupItemSaveData> SavedPickupItems;

	// All handles that are currently loading pickup items' classes
	TSet<TSharedPtr<FStreamableHandle>> LoadPickupItemsClassesHandles;

	// Spawns the pickup item at the given transform and initializes it with the given item instance save data
	void OnPickupItemClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle, const FTransform SpawnTransform,
		const FInventoryTypedSlotSaveData ItemInstanceSaveData);
};