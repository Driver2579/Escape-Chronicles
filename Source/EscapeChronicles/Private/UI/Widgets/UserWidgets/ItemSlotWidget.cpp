// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ItemSlotWidget.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Blueprint/DragDropOperation.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"

void UItemSlotWidget::SetAssociate(const FInventorySlot* InventorySlot, const int32 InAssociatedSlotIndex)
{
#if DO_CHECK
	check(InventorySlot);
#endif

	AssociatedInventorySlot = InventorySlot;
	AssociatedSlotIndex = InAssociatedSlotIndex;

	// If nullptr is passed, then this slot is empty
	if (!IsValid(InventorySlot->Instance))
	{
		ItemInstanceIcon->SetBrushFromTexture(Data->EmptySlotTexture);

		return;
	}

	const UIconInventoryItemFragment* IconFragment =
		InventorySlot->Instance->GetFragmentByClass<UIconInventoryItemFragment>();

	// If an item doesn't have an icon, give it an invalid icon
	ensureAlways(IsValid(IconFragment)) ?
		ItemInstanceIcon->SetBrushFromTexture(IconFragment->GetIcon()):
		ItemInstanceIcon->SetBrushFromTexture(Data->InvalidItemInstanceIconTexture);
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// === This code is like UWidgetBlueprintLibrary::DetectDragIfPressed but optimized for the project's task ===

	// Drag works either with DragAndDropKey or with a finger (for phones)
	if (InMouseEvent.GetEffectingButton() != Data->DragAndDropKey && !InMouseEvent.IsTouchEvent())
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const TSharedPtr<SWidget> SlateWidgetDetectingDrag = this->GetCachedWidget();

	if (!ensureAlways(SlateWidgetDetectingDrag.IsValid()))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// Sends a message that a drag has started
	return FEventReply().NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), Data->DragAndDropKey);
}

void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>(GetTransientPackage(),
		UDragDropOperation::StaticClass());

	if (!ensureAlways(IsValid(DragDropOperation)) || !ensureAlways(IsValid(Data->DragVisualWidget)))
	{
		return;
	}

	// === Visualisation ===
	Data->DragVisualWidget->SetBrush(ItemInstanceIcon->GetBrush());
	DragDropOperation->DefaultDragVisual = Data->DragVisualWidget;
	DragDropOperation->Pivot = EDragPivot::CenterCenter;

	ApplyDragVisualStyle();
	// === Applying ===

	DragDropOperation->Payload = this;
	OutOperation = DragDropOperation;
}

void UItemSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	ResetDragVisualStyle();
}

bool UItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const bool DefaultResult = Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// === Reset drag visual style on dropped widget ===
	UItemSlotWidget* DroppedSlotWidget = Cast<UItemSlotWidget>(InOperation->Payload);

	if (!ensureAlways(IsValid(DroppedSlotWidget)))
	{
		return DefaultResult;
	}

	DroppedSlotWidget->ResetDragVisualStyle();

	const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return DefaultResult;
	}

	UInventoryManagerComponent* InventoryManagerComponent = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(InventoryManagerComponent)))
	{
		return DefaultResult;
	}
	

	UInventoryManagerTransferItemsFragment* InventoryManagerTransferItemsFragment =
		InventoryManagerComponent->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

	if (!ensureAlways(IsValid(InventoryManagerTransferItemsFragment)))
	{
		return DefaultResult;
	}

	// === ===

	const FInventorySlot* FromAssociatedInventorySlot = DroppedSlotWidget->GetAssociatedInventorySlot();

#if DO_CHECK
	check(FromAssociatedInventorySlot);
	check(AssociatedInventorySlot);
#endif

	// === ===

	const FInventorySlotsArray* FromInventorySlotsArray = FromAssociatedInventorySlot->GetInventorySlotsArray();
	const FInventorySlotsArray* ToInventorySlotsArray = AssociatedInventorySlot->GetInventorySlotsArray();

#if DO_CHECK
	check(FromInventorySlotsArray);
	check(ToInventorySlotsArray);
#endif

	// === ===

	const FInventorySlotsTypedArray* FromInventorySlotsTypedArray =
		FromInventorySlotsArray->GetInventorySlotsTypedArray();

	const FInventorySlotsTypedArray* ToInventorySlotsTypedArray =
		ToInventorySlotsArray->GetInventorySlotsTypedArray();

#if DO_CHECK
	check(FromInventorySlotsTypedArray);
	check(ToInventorySlotsTypedArray);
#endif

	// === ===

	const FInventorySlotsTypedArrayContainer* FromInventorySlotsTypedArrayContainer =
		FromInventorySlotsTypedArray->GetInventorySlotsTypedArrayContainer();

	const FInventorySlotsTypedArrayContainer* ToInventorySlotsTypedArrayContainer =
		ToInventorySlotsTypedArray->GetInventorySlotsTypedArrayContainer();

#if DO_CHECK
	check(FromInventorySlotsTypedArrayContainer);
	check(ToInventorySlotsTypedArrayContainer);
#endif

	// === ===

	const UInventoryManagerComponent* FromInventoryManagerComponent =
		FromInventorySlotsTypedArrayContainer->GetInventoryManagerComponent();

	const UInventoryManagerComponent* ToInventoryManagerComponent =
		ToInventorySlotsTypedArrayContainer->GetInventoryManagerComponent();

#if DO_CHECK
	check(FromInventoryManagerComponent);
	check(ToInventoryManagerComponent);
#endif

	// === ===

	FTransferItemsData TransferItemsData;

	TransferItemsData.FromInventoryManager = FromInventoryManagerComponent;
	TransferItemsData.FromSlotTypeTag = FromInventorySlotsTypedArray->TypeTag;
	TransferItemsData.FromSlotIndex = DroppedSlotWidget->GetAssociatedSlotIndex();

	TransferItemsData.ToInventoryManager = ToInventoryManagerComponent;
	TransferItemsData.ToSlotTypeTag = ToInventorySlotsTypedArray->TypeTag;;
	TransferItemsData.ToSlotIndex = GetAssociatedSlotIndex();

	InventoryManagerTransferItemsFragment->Server_TransferItems(TransferItemsData);

	return true;
}
