// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ItemSlotWidget.h"
#include "Common/Structs/FastArraySerializers/InventorySlotsArray.h"
#include "Components/Image.h"
#include "Components/StackBox.h"
#include "ItemSlotsWidget.generated.h"

// TODO: do comments
UCLASS()
class ESCAPECHRONICLES_API UItemSlotsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetAssociatedSlotTypeTag() const { return AssociatedSlotTypeTag; }
	
	void SetInventorySlots(const TArray<FInventorySlot>& InventorySlots)
	{
		const int32 InventorySlotsNumber = InventorySlots.Num();
		
		if (InventorySlotsNumber != SlotsContainer->GetChildrenCount())
		{
			ConstructSlots(InventorySlots);

			return;
		}

		for (int32 Index = 0; Index < InventorySlotsNumber; ++Index)
		{
			check(Index >= 0 && Index < SlotsContainer->GetChildrenCount() && Index < InventorySlots.Num());

			UItemSlotWidget* ItemSlotWidget = Cast<UItemSlotWidget>(SlotsContainer->GetChildAt(Index));

			ItemSlotWidget->SetAssociate(&InventorySlots[Index], Index);
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
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UStackBox> SlotsContainer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemSlotWidget> SlotClass;

	FGameplayTag AssociatedSlotTypeTag;
};
