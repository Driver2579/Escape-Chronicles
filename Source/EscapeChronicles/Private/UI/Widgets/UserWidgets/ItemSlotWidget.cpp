// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ItemSlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

void UItemSlotWidget::SetItemInstance(UInventoryItemInstance* ItemInstance)
{
	CachedItemInstance = ItemInstance;
	
	// If nullptr is passed, then this slot is empty
	if (ItemInstance == nullptr || !ensureAlways(IsValid(ItemInstance)))
	{
		ItemInstanceIcon->SetBrushFromTexture(EmptySlotTexture);

		return;
	}
		
	const UIconInventoryItemFragment* IconFragment =
		ItemInstance->GetFragmentByClass<UIconInventoryItemFragment>();

	// If an item doesn't have an icon, give it an invalid icon
	ensureAlways(IsValid(IconFragment)) ?
		ItemInstanceIcon->SetBrushFromTexture(IconFragment->GetIcon()):
		ItemInstanceIcon->SetBrushFromTexture(InvalidItemInstanceIconTexture);
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// === This code is like UWidgetBlueprintLibrary::DetectDragIfPressed but optimized for the project's task ===

	// Drag works either with LMB or with a finger (for phones)
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton && !InMouseEvent.IsTouchEvent())
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const TSharedPtr<SWidget> SlateWidgetDetectingDrag = this->GetCachedWidget();

	if (!ensureAlways(SlateWidgetDetectingDrag.IsValid()))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// Sends a message that a drag has started
	return FEventReply().NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), EKeys::LeftMouseButton);
}

void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	const UClass* DragDropOperationClass = UDragDropOperation::StaticClass();
	
	if (!ensureAlways(DragDropOperationClass))
	{
		return;
	}
	
	UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>(GetTransientPackage(),
		DragDropOperationClass);

	if (!ensureAlways(IsValid(DragDropOperation)))
	{
		return;
	}

	// Create DragVisualWidget once, so as not to create garbage widgets for GC
	if (!IsValid(DragVisualWidget))
	{
		DragVisualWidget = NewObject<UImage>(this);
	}

	if (!ensureAlways(IsValid(DragVisualWidget)))
	{
		return;
	}

	// === Visualisation ===
	
	DragVisualWidget->SetBrush(ItemInstanceIcon->GetBrush());
	
	DragDropOperation->DefaultDragVisual = DragVisualWidget;
	DragDropOperation->Pivot = EDragPivot::CenterCenter;
	
	ItemInstanceIcon->SetVisibility(ESlateVisibility::Hidden);

	// === Applying ===
	
	DragDropOperation->Payload = CachedItemInstance.Get();
	OutOperation = DragDropOperation;
}

void UItemSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	// Return the slot to its original state as we changed it when dragging it
	ItemInstanceIcon->SetVisibility(ESlateVisibility::Visible);
}

bool UItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UInventoryItemInstance* DroppedItemInstance = Cast<UInventoryItemInstance>(InOperation->Payload);

	if (!ensureAlways(IsValid(DroppedItemInstance)))
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}
	
	
	

	return true;
}
