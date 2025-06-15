// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ItemSlotWidget.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Common/DataAssets/ItemSlotWidgetStyle.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"

void UItemSlotWidget::SetItemInstance(UInventoryItemInstance* ItemInstance)
{
	CachedItemInstance = ItemInstance;

	// If nullptr is passed, then this slot is empty
	if (ItemInstance == nullptr)
	{
		ItemInstanceIcon->SetBrushFromTexture(Style->EmptySlotTexture);

		return;
	}

#if DO_CHECK
	check(IsValid(ItemInstance));
#endif

	const UIconInventoryItemFragment* IconFragment = ItemInstance->GetFragmentByClass<UIconInventoryItemFragment>();

	// If an item doesn't have an icon, give it an invalid icon
	ensureAlways(IsValid(IconFragment)) ?
		ItemInstanceIcon->SetBrushFromTexture(IconFragment->GetIcon()):
		ItemInstanceIcon->SetBrushFromTexture(Style->InvalidItemInstanceIconTexture);
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// === This code is like UWidgetBlueprintLibrary::DetectDragIfPressed but optimized for the project's task ===

	// Drag works either with DragAndDropKey or with a finger (for phones)
	if (InMouseEvent.GetEffectingButton() != Style->DragAndDropKey && !InMouseEvent.IsTouchEvent())
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const TSharedPtr<SWidget> SlateWidgetDetectingDrag = this->GetCachedWidget();

	if (!ensureAlways(SlateWidgetDetectingDrag.IsValid()))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// Sends a message that a drag has started
	return FEventReply().NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), Style->DragAndDropKey);
}

void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>(GetTransientPackage(),
		UDragDropOperation::StaticClass());

	if (!ensureAlways(IsValid(DragDropOperation)) || !ensureAlways(IsValid(Style->DragVisualWidget)))
	{
		return;
	}

	// === Visualisation ===
	Style->DragVisualWidget->SetBrush(ItemInstanceIcon->GetBrush());
	DragDropOperation->DefaultDragVisual = Style->DragVisualWidget;
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
	const bool DefaultResult = Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	/*UInventoryItemInstance* DroppedItemInstance = Cast<UInventoryItemInstance>(InOperation->Payload);

	if (!ensureAlways(IsValid(DroppedItemInstance)))
	{
		return DefaultResult;
	}
	
	AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return DefaultResult;
	}
	
	UInventoryManagerComponent* InventoryManagerComponent = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(InventoryManagerComponent)))
	{
		return DefaultResult;
	}*/


	return DefaultResult;
}
