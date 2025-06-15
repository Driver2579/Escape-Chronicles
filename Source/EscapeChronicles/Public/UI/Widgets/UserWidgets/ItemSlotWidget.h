// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Common/DataAssets/ItemSlotWidgetData.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "ItemSlotWidget.generated.h"

struct FInventorySlot;
class UItemSlotWidgetData;

// TODO: do comments
UCLASS()
class ESCAPECHRONICLES_API UItemSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	//const FGameplayTag& GetAssociatedSlotTypeTag() const { return OuterItemSlotsWidget->GetAssociatedSlotTypeTag(); }

	int32 GetAssociatedSlotIndex() const { return AssociatedSlotIndex; }
	const FInventorySlot* GetAssociatedInventorySlot() const { return AssociatedInventorySlot; }

	void SetAssociate(const FInventorySlot* InventorySlot, const int32 InAssociatedSlotIndex);

	virtual void ApplyDragVisualStyle() { ItemInstanceIcon->SetOpacity(Data->DragIconOpacity); }
	virtual void ResetDragVisualStyle() { ItemInstanceIcon->SetOpacity(1); }

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
	TObjectPtr<UItemSlotWidgetData> Data;

	int32 AssociatedSlotIndex;
	const FInventorySlot* AssociatedInventorySlot;
};
