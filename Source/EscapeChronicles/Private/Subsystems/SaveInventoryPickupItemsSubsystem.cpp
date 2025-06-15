// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SaveInventoryPickupItemsSubsystem.h"

#include "EngineUtils.h"
#include "Actors/EscapeChroniclesInventoryPickupItem.h"
#include "Common/Structs/SaveData/InventorySlotsSaveData.h"
#include "Common/Structs/SaveData/PickupItemSaveData.h"
#include "Engine/AssetManager.h"

void USaveInventoryPickupItemsSubsystem::RegisterPickupItem(AEscapeChroniclesInventoryPickupItem* PickupItem)
{
#if DO_CHECK
	check(IsValid(PickupItem));
#endif

	// Don't do anything on clients
	if (!PickupItem->HasAuthority())
	{
		return;
	}

	PickupItems.Add(PickupItem);

	// Listen for the pickup item being destroyed to remove it from the set once it happens
	PickupItem->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnPickupItemDestroyed);
}

void USaveInventoryPickupItemsSubsystem::OnPickupItemDestroyed(AActor* DestroyedActor)
{
#if DO_CHECK
	check(IsValid(DestroyedActor));
	check(DestroyedActor->IsA<AEscapeChroniclesInventoryPickupItem>());
#endif

	PickupItems.Remove(CastChecked<AEscapeChroniclesInventoryPickupItem>(DestroyedActor));
}

void USaveInventoryPickupItemsSubsystem::OnPreSaveObject()
{
	// Empty the saved pickup items before saving the new data
	SavedPickupItems.Empty();

	// Save every pickup item currently spawned in the world
	for (const AEscapeChroniclesInventoryPickupItem* PickupItem : PickupItems)
	{
#if DO_CHECK
		check(IsValid(PickupItem));
#endif

		UInventoryItemInstance* ItemInstance = PickupItem->GetItemInstance();

		// Skip saving a pickup item if it doesn't have a valid item instance
		if (!ensureAlways(IsValid(ItemInstance)) || !ensureAlways(ItemInstance->IsInitialized()))
		{
			continue;
		}

		// Allocate the new save data for the pickup item and add it to the saved pickup items array
		FPickupItemSaveData& SavedPickupItem = SavedPickupItems.AddDefaulted_GetRef();

		// Save the pickup item's class and transform
		SavedPickupItem.PickupItemClass = PickupItem->GetClass();
		SavedPickupItem.PickupTransform = PickupItem->GetTransform();

		// Save the item instance's data
		SavedPickupItem.ItemInstanceSaveData.DefinitionClass = ItemInstance->GetDefinition();
		SavedPickupItem.ItemInstanceSaveData.InstanceStats = ItemInstance->GetInstanceStats_Mutable();
	}
}

void USaveInventoryPickupItemsSubsystem::OnPostLoadObject()
{
	/**
	 * Destroy all currently spawned pickup items in the world because we are going to respawn them using the loaded
	 * data.
	 */
	for (TActorIterator<AEscapeChroniclesInventoryPickupItem> It(GetWorld()); It; ++It)
	{
#if DO_CHECK
		check(IsValid(*It));
#endif

		It->Destroy();
	}

	// Cancel loading any pickup items' classes that we were loading before the game was loaded
	for (const TSharedPtr<FStreamableHandle>& LoadPickupItemClassHandle : LoadPickupItemsClassesHandles)
	{
#if DO_CHECK
		check(LoadPickupItemClassHandle.IsValid());
#endif

		LoadPickupItemClassHandle->CancelHandle();
	}

	// Clear the list of handles that were loading pickup items' classes
	LoadPickupItemsClassesHandles.Empty();

	for (const FPickupItemSaveData& SavedPickupItem : SavedPickupItems)
	{
		// Asynchronously load the pickup item class to spawn it in the world and pass the parameters to load its data
		TSharedPtr<FStreamableHandle> LoadPickupItemClassHandle =
			UAssetManager::GetStreamableManager().RequestAsyncLoad(SavedPickupItem.PickupItemClass.ToSoftObjectPath(),
				FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnPickupItemClassLoaded,
					SavedPickupItem.PickupTransform, SavedPickupItem.ItemInstanceSaveData));

		/**
		 * Remember the handle to release it later if needed. But only if the pickup item class isn't already loaded. If
		 * it's already loaded, then OnPickupItemClassLoaded function was already called for it too where it already
		 * released the handle.
		 */
		if (!SavedPickupItem.PickupItemClass.IsValid())
		{
			LoadPickupItemsClassesHandles.Add(LoadPickupItemClassHandle);
		}
	}
}

// ReSharper disable CppPassValueParameterByConstReference
void USaveInventoryPickupItemsSubsystem::OnPickupItemClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
	const FTransform SpawnTransform, const FInventoryTypedSlotSaveData ItemInstanceSaveData)
{
#if DO_CHECK
	check(LoadObjectHandle.IsValid());
#endif

	UClass* PickupItemClass = LoadObjectHandle->GetLoadedAsset<UClass>();

#if DO_CHECK
	check(IsValid(PickupItemClass));
	check(PickupItemClass->IsChildOf(AEscapeChroniclesInventoryPickupItem::StaticClass()));

	check(!ItemInstanceSaveData.DefinitionClass.IsNull());
#endif

	/**
	 * Spawn the pickup item actor using the loaded class and the saved transform. We use a deferred spawning because
	 * the item instance must be set before the BeginPlay is called on that actor. We also use
	 * AdjustIfPossibleButAlwaysSpawn here, because if an item was there before saving the game, then we must spawn it
	 * at the same place no matter what. If there is some kind of new collision that wasn't there before saving the
	 * game, then at least try to adjust the spawn transform.
	 */
	AEscapeChroniclesInventoryPickupItem* InventoryPickupItem =
		GetWorld()->SpawnActorDeferred<AEscapeChroniclesInventoryPickupItem>(PickupItemClass, SpawnTransform,
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

#if DO_CHECK
	check(IsValid(InventoryPickupItem));
#endif

	// Construct a new item instance to fill it with the loaded data
	UInventoryItemInstance* ItemInstance = NewObject<UInventoryItemInstance>(this);

	// Set the loaded stats and definition class to the item instance
	ItemInstance->GetInstanceStats_Mutable() = ItemInstanceSaveData.InstanceStats;
	ItemInstance->Initialize(ItemInstanceSaveData.DefinitionClass.LoadSynchronous());

	// Set the item instance we created using the loaded data to the pickup item actor
	InventoryPickupItem->SetItemInstance(ItemInstance);

	// Finish the pickup item's spawning since we finished initializing it with the loaded item instance
	InventoryPickupItem->FinishSpawning(SpawnTransform);

	/**
	 * Remove the handle from the list of handles that are currently loading pickup items' classes and release it
	 * because we don't need it anymore.
	 */
	LoadPickupItemsClassesHandles.Remove(LoadObjectHandle);
	LoadObjectHandle->ReleaseHandle();
}
// ReSharper restore CppPassValueParameterByConstReference