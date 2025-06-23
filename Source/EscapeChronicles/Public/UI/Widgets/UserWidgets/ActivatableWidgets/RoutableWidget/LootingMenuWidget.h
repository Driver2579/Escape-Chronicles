// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "LootingMenuWidget.generated.h"

class UCommonButtonBase;
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

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UItemSlotsWidget> LootingClothesWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonButtonBase> ExitButton;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag OwningMainInventoryTypeTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag LootingMainInventoryTypeTag;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag LootingClothesInventoryTypeTag;

	TWeakObjectPtr<UInventoryManagerComponent> OwningInventoryManager;
	TWeakObjectPtr<UInventoryManagerComponent> LootingInventoryManager;

	void UpdateOwningInventoryWidget() const;
	void UpdateLootingInventoryWidget() const;
};
