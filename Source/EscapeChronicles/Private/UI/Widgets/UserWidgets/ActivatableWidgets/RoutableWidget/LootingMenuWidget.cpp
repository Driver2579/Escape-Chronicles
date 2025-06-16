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

	UInventoryManagerComponent* InOwningInventoryManager = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(InOwningInventoryManager)))
	{
		return;
	}

	const UInventoryManagerTransferItemsFragment* InventoryManagerTransferItemsFragment =
		InOwningInventoryManager->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();
	
	if (!ensureAlways(IsValid(InventoryManagerTransferItemsFragment)))
	{
		return;
	}

	UInventoryManagerComponent* InLootingInventoryManager = InventoryManagerTransferItemsFragment->GetLootInventory();
	
	OwningInventoryManager = InOwningInventoryManager;
	LootingInventoryManager = InLootingInventoryManager;

	OwningInventoryManager->OnContentChanged.AddUObject(this, &ThisClass::OnOwningInventoryContentChanged);
	LootingInventoryManager->OnContentChanged.AddUObject(this, &ThisClass::OnLootingInventoryContentChanged);

	UpdateOwningInventoryWidget(OwningMainInventoryTypeTag);
	UpdateLootingInventoryWidget(LootingMainInventoryTypeTag);
}

void ULootingMenuWidget::UpdateOwningInventoryWidget(const FGameplayTag& InventoryTypeTag) const
{
	if (!ensureAlways(OwningInventoryManager.IsValid()))
	{
		return;
	}

	const FInventorySlotsTypedArrayContainer& InventoryContent = OwningInventoryManager->GetInventoryContent();

	const int32 InventoryIndex = InventoryContent.IndexOfByTag(InventoryTypeTag);

	if (ensureAlways(InventoryIndex >= 0 && InventoryIndex < InventoryContent.GetItems().Num()))
	{
		OwningInventoryWidget->SetAssociate(&InventoryContent[InventoryIndex]);
	}
}

void ULootingMenuWidget::UpdateLootingInventoryWidget(const FGameplayTag& InventoryTypeTag) const
{
	if (!ensureAlways(LootingInventoryManager.IsValid()))
	{
		return;
	}

	const FInventorySlotsTypedArrayContainer& InventoryContent = LootingInventoryManager->GetInventoryContent();

	const int32 InventoryIndex = InventoryContent.IndexOfByTag(InventoryTypeTag);

	if (ensureAlways(InventoryIndex >= 0 && InventoryIndex < InventoryContent.GetItems().Num()))
	{
		LootingInventoryWidget->SetAssociate(&InventoryContent[InventoryIndex]);
	}
}
