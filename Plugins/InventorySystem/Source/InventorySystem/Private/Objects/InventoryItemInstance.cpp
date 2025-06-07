// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragment.h"

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Definition);
	DOREPLIFETIME(ThisClass, LocalData);
}

void UInventoryItemInstance::Initialize(const TSubclassOf<UInventoryItemDefinition>& InDefinition)
{
	if (!ensureAlwaysMsgf(!bInitialized, TEXT("The instance must only be initialized once!")))
	{
		return;
	}

	if (InDefinition != nullptr)
	{
		Definition = InDefinition;
	}

#if DO_CHECK
	check(Definition);
#endif

	// === Notify fragments of the newly created item instance ===

	const UInventoryItemDefinition* DefinitionDefaultObject = Definition->GetDefaultObject<UInventoryItemDefinition>();

	for (UInventoryItemFragment* Fragment : DefinitionDefaultObject->GetFragments())
	{
		Fragment->OnItemInstanceInitialized(this);
	}

	bInitialized = true;
}

UInventoryItemInstance* UInventoryItemInstance::Duplicate(UObject* Outer) const
{
	if (!IsValid(Outer))
	{
		return nullptr;
	}

	UInventoryItemInstance* NewItemInstance = NewObject<UInventoryItemInstance>(Outer);

#if DO_CHECK
	check(NewItemInstance);
#endif

	// Copy LocalData
	for (const FLocalDataItem& Data : LocalData.GetAllData())
	{
		NewItemInstance->LocalData.SetData(Data);
	}

	NewItemInstance->Initialize(GetDefinition());

	return NewItemInstance;
}
