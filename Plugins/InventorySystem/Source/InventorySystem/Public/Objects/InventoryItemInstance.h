// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.h"
#include "Common/Structs/FastArraySerializers/LocalData.h"
#include "InventoryItemInstance.generated.h"

class UInventoryItemDefinition;
class UInventoryItemFragment;

// A specific object whose properties are described by a UInventoryItemDefinition
UCLASS(EditInlineNew)
class INVENTORYSYSTEM_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * You must call this method once before the user can interact with it in any way (e.g. in BeginPlay or before the
	 * item is displayed in the inventory)
	 */
	void Initialize(const TSubclassOf<UInventoryItemDefinition>& InDefinition = nullptr);

	bool IsInitialized() const { return bInitialized; }

	TSubclassOf<UInventoryItemDefinition> GetDefinition() const { return Definition; }

	// Returns the first fragment of type T, or nullptr if none exists
	template<typename T>
	T* GetFragmentByClass() const;

	// Overwrites OutFragments with all fragments of type T
	template<typename T>
	void GetFragmentsByClass(TArray<T*>& OutFragments) const;

	// Creates a new initialized item instance with the same Definition and LocalData
	UInventoryItemInstance* Duplicate(UObject* Outer) const;
	
	/**
	 * Breaks the object.
	 * @see for exactly how the item will break is used by Outer. The item can't break without it!
	 * @warning Outer must implement IStoringItemInstances!
	 */
	void Break();
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Determines what the item can do (Can be thrown away, is a tool, key, etc.)
	DECLARE_DELEGATE_OneParam(FOnShouldBeBroken, UInventoryItemInstance*);
	
	FOnShouldBeBroken OnShouldBeRemoved;
	
	UPROPERTY(EditAnywhere, Replicated)
	TSubclassOf<UInventoryItemDefinition> Definition;

	// This is where the fragments of definitions save the data that is needed specifically for this item
	UPROPERTY(EditAnywhere, Replicated)
	FLocalData LocalData;

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

	if (!IsValid(DefinitionDefaultObject))
	{
		return nullptr;
	}

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

	if (!IsValid(DefinitionDefaultObject))
	{
		return;
	}
		
	for (UInventoryItemFragment* Fragment : DefinitionDefaultObject->GetFragments())
	{
		T* CastedFragment = Cast<T>(Fragment);
            
		if (IsValid(CastedFragment))
		{
			OutFragments.Add(CastedFragment);
		}
	}
}