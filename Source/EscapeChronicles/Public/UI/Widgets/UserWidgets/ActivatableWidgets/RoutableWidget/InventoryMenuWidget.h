// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "UI/Widgets/UserWidgets/ItemSlotsWidget.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "InventoryMenuWidget.generated.h"

class UCommonButtonBase;

// Inventory menu screen widget
UCLASS()
class ESCAPECHRONICLES_API UInventoryMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UItemSlotsWidget> MainInventoryWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UItemSlotsWidget> ClothesInventoryWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonButtonBase> ExitButton;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag MainInventoryTypeTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag ClothesInventoryTypeTag;

	TWeakObjectPtr<UInventoryManagerComponent> CachedInventoryManager;

	void OnInventoryContentChanged() const
	{
		UpdateInventoryWidget(MainInventoryWidget, MainInventoryTypeTag);
		UpdateInventoryWidget(ClothesInventoryWidget, ClothesInventoryTypeTag);
	}

	void UpdateInventoryWidget(UItemSlotsWidget* InventoryWidget, const FGameplayTag& InventoryTypeTag) const;
};
