// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragment.h"

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Definition);
	DOREPLIFETIME(ThisClass, InstanceStats);
}

void UInventoryItemInstance::Initialize(const TSubclassOf<UInventoryItemDefinition>& InDefinition)
{
#if DO_ENSURE
	ensureAlwaysMsgf(!bInitialized, TEXT("The instance must only be initialized once!"));
#endif

	if (IsValid(InDefinition))
	{
		Definition = InDefinition;
	}

#if DO_CHECK
	check(IsValid(Definition));
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
	check(IsValid(NewItemInstance));
#endif

	// Copy FInstanceStats
	for (const FInstanceStatsItem& Data : InstanceStats.GetAllStats())
	{
		NewItemInstance->InstanceStats.SetData(Data);
	}

	NewItemInstance->Initialize(GetDefinition());

	return NewItemInstance;
}
