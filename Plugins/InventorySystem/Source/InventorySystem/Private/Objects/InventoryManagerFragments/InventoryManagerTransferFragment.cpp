// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/UnrealNetwork.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"

void UInventoryManagerTransferItemsFragment::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, OnInventoryAccessChanged);
	DOREPLIFETIME(ThisClass, Access);
}

bool UInventoryManagerTransferItemsFragment::HasInventoryAccess(
	const UInventoryManagerTransferItemsFragment* InInventoryManagerTransferItemsFragment) const
{
	return GetAccess() != EInventoryAccess::Private;
}

void UInventoryManagerTransferItemsFragment::TrySetLootInventory(UInventoryManagerComponent* InInventory)
{
	if (!ensureAlways(GetInventoryManager()->GetOwner()->HasAuthority()))
	{
		return;
	}

	if (InInventory == nullptr)
	{
		SetLootInventory(nullptr);
 
		return;
	}
	
	UInventoryManagerTransferItemsFragment* InInventoryManagerTransferItemsFragment =
		InInventory->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

	const bool bCanSetLootInventory = ensureAlways(IsValid(InInventoryManagerTransferItemsFragment)) &&
		ensureAlways(InInventoryManagerTransferItemsFragment->HasInventoryAccess(this));

	if (!bCanSetLootInventory)
	{
		return;
	}

	SetLootInventory(InInventory);

	FDelegateHandle DelegateHandle = InInventoryManagerTransferItemsFragment->OnAccessChanged.AddWeakLambda(this,
		[this, InInventoryManagerTransferItemsFragment, DelegateHandle](EInventoryAccess InInventoryAccess)
		{
			if (!InInventoryManagerTransferItemsFragment->HasInventoryAccess(this))
			{
				SetLootInventory(nullptr);
				
				InInventoryManagerTransferItemsFragment->OnAccessChanged.Remove(DelegateHandle);
			}
		});
}

bool UInventoryManagerTransferItemsFragment::Server_TransferItems_Validate(const FTransferItemsData& TransferItemsData)
{
	const UInventoryManagerComponent* FromInventoryManager = TransferItemsData.FromInventoryManager.Get();
	const UInventoryManagerComponent* ToInventoryManager = TransferItemsData.ToInventoryManager.Get();

	const UInventoryManagerTransferItemsFragment* FromInventoryManagerTransferItemsFragment =
		FromInventoryManager->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

	const UInventoryManagerTransferItemsFragment* ToInventoryManagerTransferItemsFragment =
		ToInventoryManager->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

	const UInventoryManagerComponent* Owner = GetInventoryManager();

	const bool bSuccessValidation = ensureAlways(IsValid(FromInventoryManagerTransferItemsFragment)) &&
		ensureAlways(IsValid(FromInventoryManagerTransferItemsFragment)) && ensureAlways(IsValid(Owner));

	if (!bSuccessValidation)
	{
		return false;
	}

	if (Owner == FromInventoryManager)
	{
		if (Owner == ToInventoryManager || ToInventoryManagerTransferItemsFragment->HasInventoryAccess(this))
		{
			return true;
		}
	}
	else if (Owner == ToInventoryManager)
	{
		if (Owner == FromInventoryManager || FromInventoryManagerTransferItemsFragment->HasInventoryAccess(this))
		{
			return true;
		}
	}

	return false;
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
