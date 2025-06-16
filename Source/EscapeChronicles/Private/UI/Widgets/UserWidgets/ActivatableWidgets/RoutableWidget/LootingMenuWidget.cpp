// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/LootingMenuWidget.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsTypedArrayContainer.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"
#include "UI/Widgets/UserWidgets/ItemSlotsWidget.h"

void ULootingMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	UInventoryManagerComponent* InventoryManager = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(InventoryManager)))
	{
		return;
	}

	UInventoryManagerTransferItemsFragment* InventoryManagerTransferItemsFragment =
		InventoryManager->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();
	
	if (!ensureAlways(IsValid(InventoryManagerTransferItemsFragment)))
	{
		return;
	}

	//InventoryManagerTransferItemsFragment->Get
	OwningInventoryManager = InventoryManager;

	/*InventoryManager->OnContentChanged.AddUObject(this, &ThisClass::OnOwningInventoryContentChanged);

	UpdateOwningInventoryWidget(OwningMainInventoryTypeTag);
	UpdateLootingInventoryWidget(LootingMainInventoryTypeTag);*/
}
