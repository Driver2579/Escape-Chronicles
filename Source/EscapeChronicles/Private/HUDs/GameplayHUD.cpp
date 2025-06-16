// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDs/GameplayHUD.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"

void AGameplayHUD::BeginPlay()
{
	Super::BeginPlay();

	AEscapeChroniclesCharacter* Character = GetOwner<AEscapeChroniclesCharacter>();

	/*if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	UInventoryManagerTransferItemsFragment* InventoryManagerTransferItemsFragment
		= Inventory->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

	if (!ensureAlways(IsValid(InventoryManagerTransferItemsFragment)))
	{
		return;
	}

	InventoryManagerTransferItemsFragment->OnLootInventoryChanged.AddUObject(this, & ThisClass::OnLootInventoryChanged);*/
}

void AGameplayHUD::OnLootInventoryChanged(UInventoryManagerComponent* InInventory)
{
	
}
