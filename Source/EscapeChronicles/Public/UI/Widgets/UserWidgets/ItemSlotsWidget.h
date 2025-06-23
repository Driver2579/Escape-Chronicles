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

	void SetAssociate(const FInventorySlotsTypedArray* InventorySlotsTypedArray);

protected:
	virtual void ConstructSlots(const TArray<FInventorySlot>& InventorySlots);

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UStackBox> SlotsContainer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemSlotWidget> SlotClass;

	const FInventorySlotsTypedArray* AssociatedInventorySlotsTypedArray;
};
