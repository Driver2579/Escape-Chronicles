// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.h"
#include "Common/Structs/FastArraySerializers/InstanceStats.h"
#include "InventoryItemInstance.generated.h"

class UInventoryItemDefinition;
class UInventoryItemFragment;

/**
 * Represents a runtime instance of an inventory item that can be replicated across the network.
 *
 * This class encapsulates both the definition (shared/static data via UInventoryItemDefinition) and instance-specific
 * state (via InstanceStats), making it suitable for representing actual items possessed by players or AI during gameplay.
 *
 * Supports fragment-based modular data extensions and provides methods to initialize, duplicate, and query those
 * fragments. Must be explicitly initialized before use.
 *
 * Designed for multiplayer use: fully supports replication and can be managed as a subobject.
 */
UCLASS(EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool IsInitialized() const { return bInitialized; }

	/**
	 * Retrieves the first fragment of the specified class type Returns nullptr if no fragment of that type is present.
	 */
	void Initialize(const TSubclassOf<UInventoryItemDefinition>& InDefinition = nullptr);

	TSubclassOf<UInventoryItemDefinition> GetDefinition() const { return Definition; }

	FInstanceStats& GetInstanceStats_Mutable() { return InstanceStats; }
	
	// Gathers all fragments of the specified class type and writes them into the provided array.
	template<typename T>
	T* GetFragmentByClass() const;

	// Gathers all fragments of the specified class type and writes them into the provided array.
	template<typename T>
	void GetFragmentsByClass(TArray<T*>& OutFragments) const;

	/**
	 * Creates a copy of this item instance, including its stats and definition (initialized on its basis). The
	 * new instance must be assigned an appropriate outer (e.g., inventory component).
	 */
	UInventoryItemInstance* Duplicate(UObject* Outer) const;

	/**
	 * Breaks the object.
	 * @see for exactly how the item will break is used by Outer. The item can't break without it!
	 * @warning Outer must implement IStoringItemInstances!
	 */
	void Break();

private:
	// Determines what the item can do (сan be thrown away, is a tool, key, etc.)
	UPROPERTY(EditAnywhere, Replicated)
	TSubclassOf<UInventoryItemDefinition> Definition;

	// Stores per-instance data specific to this item, populated by its definition's fragments
	UPROPERTY(EditAnywhere, Replicated)
	FInstanceStats InstanceStats;

	// True if the item has been properly initialized and is ready for use
	bool bInitialized = false;
};

template<typename T>
T* UInventoryItemInstance::GetFragmentByClass() const
{
	static_assert(TIsDerivedFrom<T, UInventoryItemFragment>::Value, "T must be inherited from UInventoryItemFragment!");

	if (!Definition)
	{
		return nullptr;
	}

	const UInventoryItemDefinition* DefinitionDefaultObject = Definition->GetDefaultObject<UInventoryItemDefinition>();

	for (UInventoryItemFragment* Fragment : DefinitionDefaultObject->GetFragments())
	{
		T* CastedFragment = Cast<T>(Fragment);

		if (IsValid(CastedFragment))
		{
			return CastedFragment;
		}
	}

	return nullptr;
}

template<typename T>
void UInventoryItemInstance::GetFragmentsByClass(TArray<T*>& OutFragments) const
{
	static_assert(TIsDerivedFrom<T, UInventoryItemFragment>::Value, "T must be inherited from UInventoryItemFragment!");

	OutFragments.Empty();

	if (!Definition)
	{
		return;
	}

	const UInventoryItemDefinition* DefinitionDefaultObject = Definition->GetDefaultObject<UInventoryItemDefinition>();

	for (UInventoryItemFragment* Fragment : DefinitionDefaultObject->GetFragments())
	{
		T* CastedFragment = Cast<T>(Fragment);

		if (IsValid(CastedFragment))
		{
			OutFragments.Add(CastedFragment);
		}
	}
}