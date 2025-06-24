// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/DataAssets/ItemSlotWidgetData.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"
#include "SelectedItemSlotWidget.generated.h"

struct FInventorySlot;
class UItemSlotWidgetData;

UCLASS()
class ESCAPECHRONICLES_API USelectedItemSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override
	{
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

		UInventoryManagerSelectorFragment* SelectorFragment =
			Inventory->GetFragmentByClass<UInventoryManagerSelectorFragment>();

		if (!ensureAlways(IsValid(SelectorFragment)))
		{
			return;
		}

		SelectorFragment->OnOffsetCurrentSlotIndex.AddWeakLambda(this, [this](int32 Index)
		{
			UpdateItemInstanceIcon();
		});

		Inventory->OnContentChanged.AddWeakLambda(this, [this]
		{
			UpdateItemInstanceIcon();
		});

		Inventory->OnPreDeleteItem.AddWeakLambda(this,
			[this](int32 SlotIndex, const FGameplayTag& SlotTypeTag)
			{
				UpdateItemInstanceIcon();
			});

		UpdateItemInstanceIcon();
	}

	virtual void NativePreConstruct() override
	{
		Super::NativePreConstruct();

		if (Data)
		{
			SlotBackgroundIcon->SetBrush(Data->DefaultSlotBackgroundBrush);
			ItemInstanceIcon->SetBrush(Data->EmptySlotBrush);
		}
	}

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemInstanceIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SlotBackgroundIcon;

	UPROPERTY(EditAnywhere)
	FGameplayTag SlotsTypeTag;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UItemSlotWidgetData> Data;

	void UpdateItemInstanceIcon() const
	{
		const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();
		if (!ensureAlways(IsValid(Character))) return;

		const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();
		if (!ensureAlways(IsValid(Inventory))) return;

		const UInventoryManagerSelectorFragment* SelectorFragment =
			Inventory->GetFragmentByClass<UInventoryManagerSelectorFragment>();

		if (!ensureAlways(IsValid(SelectorFragment))) return;

		const UInventoryItemInstance* ItemInstance =
			Inventory->GetItemInstance(SelectorFragment->GetCurrentSlotIndex(), SlotsTypeTag);

		if (!IsValid(ItemInstance))
		{
			ItemInstanceIcon->SetBrush(Data->EmptySlotBrush);

			return;
		}

		const UIconInventoryItemFragment* IconFragment = ItemInstance->GetFragmentByClass<UIconInventoryItemFragment>();

		ensureAlways(IsValid(IconFragment)) ?
			ItemInstanceIcon->SetBrush(IconFragment->GetIcon()):
			ItemInstanceIcon->SetBrush(Data->InvalidItemInstanceBrush);
	}
};
