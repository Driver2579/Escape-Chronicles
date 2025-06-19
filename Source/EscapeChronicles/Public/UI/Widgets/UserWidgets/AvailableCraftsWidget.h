// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonUserWidget.h"
#include "CraftSlotWidget.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Structs/TableRowBases/InventoryManagerCraftData.h"
#include "Components/WrapBox.h"
#include "Objects/InventoryManagerFragments/InventoryManagerCraftItemsFragment.h"

#include "AvailableCraftsWidget.generated.h"

struct FInventorySlot;
class UItemSlotWidgetData;

UCLASS()
class ESCAPECHRONICLES_API UAvailableCraftsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override
	{
		Super::NativeConstruct();

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

		const UInventoryManagerCraftItemsFragment* CraftFragment =
			Inventory->GetFragmentByClass<UInventoryManagerCraftItemsFragment>();

		if (!ensureAlways(IsValid(CraftFragment)))
		{
			return;
		}

		const UDataTable* AvailableCraftList = CraftFragment->GetAvailableCraftList();

		if (!ensureAlways(IsValid(AvailableCraftList)))
		{
			return;
		}

		//TFunctionRef<void (const FName& Key, const T& Value)> Predicate
		AvailableCraftList->ForeachRow<FInventoryManagerCraftData>("",
			[this](const FName& Key, const FInventoryManagerCraftData& Value)
			{
				UCraftSlotWidget* NewSlotWidget = CreateWidget<UCraftSlotWidget>(this, SlotClass);

				SlotsContainer->AddChildToWrapBox(NewSlotWidget);

				NewSlotWidget->SetAssociate(Key, Value);
			});
	}

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UWrapBox> SlotsContainer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCraftSlotWidget> SlotClass;
};
