// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ItemSlotsWidget.h"

#include "Common/Structs/FastArraySerializers/InventorySlotsTypedArrayContainer.h"

void UItemSlotsWidget::SetAssociate(const FInventorySlotsTypedArray* InventorySlotsTypedArray)
{
	AssociatedInventorySlotsTypedArray = InventorySlotsTypedArray;

	const TArray<FInventorySlot>& Slots = InventorySlotsTypedArray->Array.GetItems();
		
	if (Slots.Num() != SlotsContainer->GetChildrenCount())
	{
		ConstructSlots(Slots);

		return;
	}

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
#if DO_CHECK
		check(Index >= 0 && Index < SlotsContainer->GetChildrenCount() && Index < Slots.Num());
#endif

		UItemSlotWidget* ItemSlotWidget = Cast<UItemSlotWidget>(SlotsContainer->GetChildAt(Index));

		ItemSlotWidget->SetAssociate(&Slots[Index], Index);
	}
}

void UItemSlotsWidget::ConstructSlots(const TArray<FInventorySlot>& InventorySlots)
{
	SlotsContainer->ClearChildren();

	for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
	{
		UItemSlotWidget* NewSlotWidget = CreateWidget<UItemSlotWidget>(this, SlotClass);

		SlotsContainer->AddChildToStackBox(NewSlotWidget);
		NewSlotWidget->SetAssociate(&InventorySlots[Index], Index);
	}
}