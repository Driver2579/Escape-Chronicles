// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/Image.h"
#include "Objects/InventoryItemInstance.h"
#include "ItemSlotWidget.generated.h"

class UItemSlotWidgetStyle;

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
	TObjectPtr<UItemSlotWidgetStyle> Style;

	TWeakObjectPtr<UInventoryItemInstance> CachedItemInstance;
};
