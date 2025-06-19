// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/CraftSlotWidget.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"

void UCraftSlotWidget::SetAssociate(const FName& InAssociatedRowName,
	const FInventoryManagerCraftData& InAssociatedRowData)
{
	AssociatedRowName = &InAssociatedRowName;
	AssociatedRowData = &InAssociatedRowData;
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

			return;
		}
	}

	ItemInstanceIcon->SetBrush(Data->InvalidItemInstanceBrush);
}

void UCraftSlotWidget::NativeOnClicked()
{
	Super::NativeOnClicked();

	
}
