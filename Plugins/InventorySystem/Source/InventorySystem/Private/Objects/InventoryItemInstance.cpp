// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragment.h"

void UInventoryItemInstance::Initialize(const TSubclassOf<UInventoryItemDefinition>& InDefinition)
{
	if (!ensureAlwaysMsgf(!bInitialized, TEXT("The instance must only be initialized once!")))
	{
		return;
	}

	if (InDefinition && ensureAlways(IsValid(InDefinition)))
	{
		Definition = InDefinition;
	}
	else if (!ensureAlwaysMsgf(Definition,
		TEXT("Definition must be valid either by InDefinition or by default before initialization!")))
	{
		return;
	}

	const UInventoryItemDefinition* DefinitionDefaultObject = Definition->GetDefaultObject<UInventoryItemDefinition>();

	if (!ensureAlways(IsValid(DefinitionDefaultObject)))
	{
		return;
	}

	for (UInventoryItemFragment* Fragment : DefinitionDefaultObject->GetFragments())
	{
		Fragment->OnInstanceInitialized(this);
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

	if (!IsValid(NewItemInstance))
	{
		return nullptr;
	}

	for (FLocalDataItem Data : LocalData.GetAllData())
	{
		NewItemInstance->LocalData.SetData(Data);
	}

	NewItemInstance->Initialize(GetDefinition());

	return NewItemInstance;
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Definition);
	DOREPLIFETIME(ThisClass, LocalData);
}
