// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/CraftSlotWidget.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"
#include "Objects/InventoryManagerFragments/InventoryManagerCraftItemsFragment.h"

void UCraftSlotWidget::SetAssociate(const FName& InAssociatedRowName,
	const FInventoryManagerCraftData& InAssociatedRowData)
{
	AssociatedRowName = &InAssociatedRowName;
	AssociatedRowData = &InAssociatedRowData;
}

void UCraftSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	Inventory->OnContentChanged.AddUObject(this, &ThisClass::UpdateEnabled);
}

void UCraftSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!AssociatedRowData)
	{
		return;
	}

	const UInventoryItemDefinition* DefinitionCDO = AssociatedRowData->ResultItemDefinition.GetDefaultObject();

	for (UInventoryItemFragment* Fragment : DefinitionCDO->GetFragments())
	{
		const UIconInventoryItemFragment* IconFragment = Cast<UIconInventoryItemFragment>(Fragment);

		if (IsValid(IconFragment))
		{
			ItemInstanceIcon->SetBrush(IconFragment->GetIcon());

			UpdateEnabled();

			return;
		}
	}

	ItemInstanceIcon->SetBrush(Data->InvalidItemInstanceBrush);

	UpdateEnabled();
}

void UCraftSlotWidget::NativeOnClicked()
{
	Super::NativeOnClicked();

	const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	UInventoryManagerCraftItemsFragment* CraftItemsFragment =
		Inventory->GetFragmentByClass<UInventoryManagerCraftItemsFragment>();

	if (!ensureAlways(IsValid(CraftItemsFragment)))
	{
		return;
	}

	CraftItemsFragment->Server_Craft(GetAssociatedRowName());
}

void UCraftSlotWidget::UpdateEnabled()
{
	const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	UInventoryManagerCraftItemsFragment* CraftItemsFragment =
		Inventory->GetFragmentByClass<UInventoryManagerCraftItemsFragment>();

	if (!ensureAlways(IsValid(CraftItemsFragment)))
	{
		return;
	}

	SetIsEnabled(CraftItemsFragment->IsCraftPossible(GetAssociatedRowName()));
}
