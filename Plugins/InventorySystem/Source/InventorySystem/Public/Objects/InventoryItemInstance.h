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
	TSubclassOf<UInventoryItemDefinition> GetDefinition() const { return Definition; }
	
	bool IsInitialized() const { return bInitialized; }

	template<typename T>
	T* GetFragmentByClass() const;

	template<typename T>
	void GetFragmentsByClass(TArray<T*>& OutFragments) const;
	
	void Initialize(const TSubclassOf<UInventoryItemDefinition>& InDefinition = nullptr);

	UInventoryItemInstance* Duplicate(UObject* Outer) const;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Break() { OnShouldBeRemoved.Execute(this); }
	
	UPROPERTY(Replicated)
	FLocalData LocalData;
	
private:
	DECLARE_DELEGATE_OneParam(FOnShouldBeBroken, UInventoryItemInstance*);
	FOnShouldBeBroken OnShouldBeRemoved;
	
	UPROPERTY(EditAnywhere, Replicated)
	TSubclassOf<UInventoryItemDefinition> Definition;
	
	bool bInitialized = false;
};

template<typename T>
T* UInventoryItemInstance::GetFragmentByClass() const
{
	static_assert(TIsDerivedFrom<T, UInventoryItemFragment>::Value, "T must be inherited from UInventoryItemFragment!");

	if (!IsValid(GetDefinition()))
	{
		return nullptr;
	}
	
	const UInventoryItemDefinition* DefinitionDefaultObject =
		GetDefinition()->GetDefaultObject<UInventoryItemDefinition>();

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

	if (!ensureAlways(IsValid(GetDefinition())))
	{
		return;
	}
	
	const UInventoryItemDefinition* DefinitionDefaultObject =
		GetDefinition()->GetDefaultObject<UInventoryItemDefinition>();

	if (!ensureAlways(IsValid(DefinitionDefaultObject)))
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