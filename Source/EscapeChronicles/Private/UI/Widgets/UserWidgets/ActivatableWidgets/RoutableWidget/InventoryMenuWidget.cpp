// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/InventoryMenuWidget.h"

#include "CommonButtonBase.h"
#include "Characters/EscapeChroniclesCharacter.h"
void UInventoryMenuWidget::NativeConstruct()
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

	UInventoryManagerComponent* InventoryManager = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(InventoryManager)))
	{
		return;
	}

	CachedInventoryManager = InventoryManager;

	InventoryManager->OnContentChanged.AddUObject(this, &ThisClass::OnInventoryContentChanged);

	UpdateInventoryWidget(MainInventoryWidget, MainInventoryTypeTag);
	UpdateInventoryWidget(ClothesInventoryWidget, ClothesInventoryTypeTag);
}

void UInventoryMenuWidget::UpdateInventoryWidget(UItemSlotsWidget* InventoryWidget,
	const FGameplayTag& InventoryTypeTag) const
{
	if (!ensureAlways(CachedInventoryManager.IsValid()))
	{
		return;
	}

	const FInventorySlotsTypedArrayContainer& InventoryContent = CachedInventoryManager->GetInventoryContent();

	const int32 InventoryIndex = InventoryContent.IndexOfByTag(InventoryTypeTag);

	if (ensureAlways(InventoryIndex >= 0 && InventoryIndex < InventoryContent.GetItems().Num()))
	{
		InventoryWidget->SetAssociate(&InventoryContent[InventoryIndex]);
	}
}
