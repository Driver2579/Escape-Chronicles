// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ItemSlotWidget.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsArray.h"
#include "Components/StackBox.h"
#include "ItemSlotsWidget.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UItemSlotsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	const FInventorySlotsTypedArray* GetAssociatedInventorySlotsTypedArray() const
	{
		return AssociatedInventorySlotsTypedArray;
	}

	void SetAssociate(const FInventorySlotsTypedArray* InventorySlotsTypedArray)
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
			check(Index >= 0 && Index < SlotsContainer->GetChildrenCount() && Index < Slots.Num());

			UItemSlotWidget* ItemSlotWidget = Cast<UItemSlotWidget>(SlotsContainer->GetChildAt(Index));

			ItemSlotWidget->SetAssociate(&Slots[Index], Index);
		}
	}

protected:
	virtual void ConstructSlots(const TArray<FInventorySlot>& InventorySlots)
	{
		SlotsContainer->ClearChildren();

		for (int32 Index = 0; Index < InventorySlots.Num(); ++Index)
		{
			UItemSlotWidget* NewSlotWidget = CreateWidget<UItemSlotWidget>(this, SlotClass);

			SlotsContainer->AddChildToStackBox(NewSlotWidget);
			NewSlotWidget->SetAssociate(&InventorySlots[Index], Index);
		}
	}

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UStackBox> SlotsContainer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemSlotWidget> SlotClass;

	const FInventorySlotsTypedArray* AssociatedInventorySlotsTypedArray;
};
