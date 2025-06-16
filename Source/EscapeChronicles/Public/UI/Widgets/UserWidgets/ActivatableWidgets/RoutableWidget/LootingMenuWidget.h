// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "LootingMenuWidget.generated.h"

class UInventoryManagerComponent;
class UItemSlotsWidget;

// Gameplay menu screen widget
UCLASS()
class ESCAPECHRONICLES_API ULootingMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UItemSlotsWidget> OwningInventoryWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UItemSlotsWidget> LootingInventoryWidget;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag OwningMainInventoryTypeTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag LootingMainInventoryTypeTag;

	TWeakObjectPtr<UInventoryManagerComponent> OwningInventoryManager;
	TWeakObjectPtr<UInventoryManagerComponent> LootingInventoryManager;

	void OnOwningInventoryContentChanged() const
	{
		UpdateOwningInventoryWidget(OwningMainInventoryTypeTag);
	}

	void OnLootingInventoryContentChanged() const
	{
		UpdateLootingInventoryWidget(LootingMainInventoryTypeTag);
	}

	void UpdateOwningInventoryWidget(const FGameplayTag& InventoryTypeTag) const;
	void UpdateLootingInventoryWidget(const FGameplayTag& InventoryTypeTag) const;
};
