// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/UnrealNetwork.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"

void UInventoryManagerTransferItemsFragment::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryAccess);
}

void UInventoryManagerTransferItemsFragment::Server_TransferItems_Implementation(
	const FTransferItemsData& TransferItemsData)
{
	UInventoryManagerComponent* FromInventoryManager = TransferItemsData.FromInventoryManager.Get();
	UInventoryManagerComponent* ToInventoryManager = TransferItemsData.ToInventoryManager.Get();

#if DO_CHECK
	check(IsValid(FromInventoryManager));
	check(IsValid(ToInventoryManager));
#endif

	const UInventoryItemInstance* FromItemInstance =
		FromInventoryManager->GetItemInstance(TransferItemsData.FromSlotIndex, TransferItemsData.FromSlotTypeTag);

	const UInventoryItemInstance* ToItemInstance =
		FromInventoryManager->GetItemInstance(TransferItemsData.ToSlotIndex, TransferItemsData.ToSlotTypeTag);

	if (IsValid(ToItemInstance))
	{
		const bool bSuccess = ensureAlways(FromInventoryManager->DeleteItem(TransferItemsData.ToSlotIndex,
				TransferItemsData.ToSlotTypeTag));

		if (!bSuccess) return;
	}
	if (IsValid(FromItemInstance))
	{
		const bool bSuccess = ensureAlways(ToInventoryManager->DeleteItem(TransferItemsData.FromSlotIndex,
			TransferItemsData.FromSlotTypeTag));

		if (!bSuccess) return;
	}

	if (IsValid(ToItemInstance))
	{
		const bool bSuccess = ensureAlways(FromInventoryManager->AddItem(ToItemInstance,
			TransferItemsData.FromSlotIndex, TransferItemsData.FromSlotTypeTag));

		if (!bSuccess) return;
	}
	if (IsValid(FromItemInstance))
	{
		ensureAlways(ToInventoryManager->AddItem(FromItemInstance,
			TransferItemsData.ToSlotIndex, TransferItemsData.ToSlotTypeTag));
	}
}
