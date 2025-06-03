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
	void SetItemInstance(UInventoryItemInstance* ItemInstance);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemInstanceIcon;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EmptySlotTexture;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> InvalidItemInstanceIconTexture;

	UPROPERTY(Transient)
	TObjectPtr<UImage> DragVisualWidget;

	TWeakObjectPtr<UInventoryItemInstance> CachedItemInstance;
};
