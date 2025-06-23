// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/LootingMenuWidget.h"

#include "CommonButtonBase.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsTypedArrayContainer.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"
#include "UI/Widgets/UserWidgets/ItemSlotsWidget.h"

void ULootingMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ExitButton->OnClicked().AddWeakLambda(this, [this]
	{
		DeactivateWidget();
	});
	
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

	OwningInventoryManager->OnContentChanged.AddUObject(this, &ThisClass::UpdateOwningInventoryWidget);
	LootingInventoryManager->OnContentChanged.AddUObject(this, &ThisClass::UpdateLootingInventoryWidget);

	UpdateOwningInventoryWidget();
	UpdateLootingInventoryWidget();
}

void ULootingMenuWidget::UpdateOwningInventoryWidget() const
{
	if (!ensureAlways(OwningInventoryManager.IsValid()))
	{
		return;
	}

	const FInventorySlotsTypedArrayContainer& InventoryContent = OwningInventoryManager->GetInventoryContent();

	const int32 InventoryIndex = InventoryContent.IndexOfByTag(OwningMainInventoryTypeTag);

	if (ensureAlways(InventoryContent.GetItems().IsValidIndex(InventoryIndex)))
	{
		OwningInventoryWidget->SetAssociate(&InventoryContent[InventoryIndex]);
	}
}

void ULootingMenuWidget::UpdateLootingInventoryWidget() const
{
	if (!ensureAlways(LootingInventoryManager.IsValid()))
	{
		return;
	}

	const FInventorySlotsTypedArrayContainer& InventoryContent = LootingInventoryManager->GetInventoryContent();

	const int32 MainInventoryIndex = InventoryContent.IndexOfByTag(LootingMainInventoryTypeTag);

	if (ensureAlways(InventoryContent.GetItems().IsValidIndex(MainInventoryIndex)))
	{
		LootingInventoryWidget->SetAssociate(&InventoryContent[MainInventoryIndex]);
	}

	const int32 ClothesInventoryIndex = InventoryContent.IndexOfByTag(LootingClothesInventoryTypeTag);

	if (InventoryContent.GetItems().IsValidIndex(ClothesInventoryIndex))
	{
		ClothesWidget->SetVisibility(ESlateVisibility::Visible);
		LootingClothesWidget->SetAssociate(&InventoryContent[ClothesInventoryIndex]);
	}
	else
	{
		ClothesWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}
