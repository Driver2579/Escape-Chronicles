// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "UI/Widgets/UserWidgets/ItemSlotsWidget.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "InventoryMenuWidget.generated.h"

// Inventory menu screen widget
UCLASS()
class ESCAPECHRONICLES_API UInventoryMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();

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
		
		FOnInventoryContentChanged::FDelegate InventoryContentChangedDelegate;
		InventoryContentChangedDelegate.BindUObject(this, &UInventoryMenuWidget::OnInventoryContentChanged);
		
		InventoryManager->AddInventoryContentChangedHandler(InventoryContentChangedDelegate);

		UpdateMainInventoryWidget();
	}

private:
	UPROPERTY(meta = (BindWidget))
	UItemSlotsWidget* MainInventoryWidget;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag MainInventoryTypeTag;

	TWeakObjectPtr<UInventoryManagerComponent> CachedInventoryManager;

	void OnInventoryContentChanged()
	{
		UpdateMainInventoryWidget();
	}

	void UpdateMainInventoryWidget() const
	{
		if (!ensureAlways(CachedInventoryManager.IsValid()))
		{
			return;
		}

		const FInventorySlotsTypedArrayContainer& InventorySlotsTypedArrayContainer =
			CachedInventoryManager->GetTypedInventorySlotsLists();
		
		const int32 InventoryIndex = InventorySlotsTypedArrayContainer.IndexOfByTag(MainInventoryTypeTag);

		check(InventoryIndex >= 0 && InventoryIndex < InventorySlotsTypedArrayContainer.GetArrays().Num());
		
		MainInventoryWidget->SetInventorySlots(InventorySlotsTypedArrayContainer[InventoryIndex].Array.GetSlots());
		
	}
};
