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
	UE_LOG(LogTemp, Error, TEXT("InventoryManagerTransferItemsFragment::Server_TransferItems_Implementation"));
}
