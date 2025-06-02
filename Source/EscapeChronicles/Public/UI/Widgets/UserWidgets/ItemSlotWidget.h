// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Objects/InventoryItemInstance.h>

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/Image.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"
#include "ItemSlotWidget.generated.h"

// TODO: do comments
UCLASS()
class ESCAPECHRONICLES_API UItemSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void SetItemInstance(const UInventoryItemInstance* ItemInstance) const
	{
		if (ItemInstance == nullptr || !ensureAlways(IsValid(ItemInstance)))
		{
			ItemInstanceIcon->SetBrushFromTexture(EmptySlotTexture);

			return;
		}
		
		const UIconInventoryItemFragment* IconFragment =
			ItemInstance->GetFragmentByClass<UIconInventoryItemFragment>();
		
		ensureAlways(IsValid(IconFragment)) ?
			ItemInstanceIcon->SetBrushFromTexture(IconFragment->GetIcon()):
			ItemInstanceIcon->SetBrushFromTexture(InvalidItemInstanceIconTexture);
	}
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemInstanceIcon;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EmptySlotTexture;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> InvalidItemInstanceIconTexture;
};
