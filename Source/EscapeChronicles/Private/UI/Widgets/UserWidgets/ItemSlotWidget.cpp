// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ItemSlotWidget.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Blueprint/DragDropOperation.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Enums/ItemClassification.h"
#include "Objects/InventoryItemFragments/ClassificationInventoryItemFragment.h"
#include "Objects/InventoryItemFragments/ContrabandBagInventoryItemFragment.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"

void UItemSlotWidget::SetAssociate(const FInventorySlot* InventorySlot, const int32 InAssociatedSlotIndex)
{
#if DO_CHECK
	check(InventorySlot);
#endif

	AssociatedInventorySlot = InventorySlot;
	AssociatedSlotIndex = InAssociatedSlotIndex;

	if (!ensureAlways(Data))
	{
		return;
	}

	// If nullptr is passed, then this slot is empty
	if (!IsValid(InventorySlot->Instance))
	{
		ItemInstanceIcon->SetBrush(Data->EmptySlotBrush);

		return;
	}

	const UIconInventoryItemFragment* IconFragment =
		InventorySlot->Instance->GetFragmentByClass<UIconInventoryItemFragment>();

	// If an item doesn't have an icon, give it an invalid icon
	ensureAlways(IsValid(IconFragment)) ?
		ItemInstanceIcon->SetBrush(IconFragment->GetIcon()):
		ItemInstanceIcon->SetBrush(Data->InvalidItemInstanceBrush);
}

void UItemSlotWidget::SetSlotSelected(const bool bInSlotSelected)
{
	if (bInSlotSelected)
	{
		SlotBackgroundIcon->SetBrush(Data->SelectedSlotBackgroundBrush);
	}
	else
	{
		SlotBackgroundIcon->SetBrush(Data->DefaultSlotBackgroundBrush);
	}

	bSlotSelected = bInSlotSelected;
}

void UItemSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Data->ToolTipWidget)
	{
		SetToolTip(Data->ToolTipWidget);
	}
}

void UItemSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Data)
	{
		SlotBackgroundIcon->SetBrush(Data->DefaultSlotBackgroundBrush);
		ItemInstanceIcon->SetBrush(Data->EmptySlotBrush);
	}
}

void UItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (bSelectSlotOnHover)
	{
		SetSlotSelected(true);

		bSlotSelectedOnHover = true;
	}

	UHintBaseWidget* ToolTip = Cast<UHintBaseWidget>(GetToolTip());

	if (!IsValid(ToolTip))
	{
		return;
	}

	if (!IsValid(AssociatedInventorySlot->Instance))
	{
		ToolTip->SetVisibility(ESlateVisibility::Hidden);

		return;
	}

	ToolTip->SetVisibility(ESlateVisibility::Visible);

	const UClassificationInventoryItemFragment* ClassificationFragment =
		AssociatedInventorySlot->Instance->GetFragmentByClass<UClassificationInventoryItemFragment>();
	
	if (IsValid(ClassificationFragment) && ClassificationFragment->GetItemClassification() == EItemClassification::Contraband)
	{
		ToolTip->GetTitleTextBlock()->SetColorAndOpacity(Data->ContrabandToolTipColor);
	}
	else
	{
		ToolTip->GetTitleTextBlock()->SetColorAndOpacity(Data->DefaultToolTipColor);
	}

	FText TitleText =
		AssociatedInventorySlot->Instance->GetDefinition()->GetDefaultObject<UInventoryItemDefinition>()->GetName();

	const FInstanceStatsItem* DurabilityStat =
		AssociatedInventorySlot->Instance->GetInstanceStats_Mutable().GetStat(Data->DurabilityTag);

	if (DurabilityStat)
	{
		TitleText = FText::Format(FText::FromString("{0} ({1})"), TitleText, DurabilityStat->Value);
	}

	ToolTip->SetTitleText(TitleText);
	ToolTip->SetMainText(FText::GetEmpty());
}

void UItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (bSelectSlotOnHover && bSlotSelectedOnHover)
	{
		SetSlotSelected(false);

		bSlotSelectedOnHover = false;
	}
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!ensureAlways(Data))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (!IsValid(AssociatedInventorySlot->Instance))
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// === This code is like UWidgetBlueprintLibrary::DetectDragIfPressed but optimized for the project's task ===

	// Drag works either with DragAndDropKey
	if (InMouseEvent.GetEffectingButton() != Data->DragAndDropKey)
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

	if (!ensureAlways(Data) || !ensureAlways(IsValid(Data->DragVisualWidget)))
	{
		return;
	}

	UDragDropOperation* DragDropOperation = NewObject<UDragDropOperation>(GetTransientPackage(),
		UDragDropOperation::StaticClass());

	if (!ensureAlways(IsValid(DragDropOperation)))
	{
		return;
	}

	// === Visualisation ===

	Data->DragVisualWidget->SetBrush(ItemInstanceIcon->GetBrush());
	DragDropOperation->DefaultDragVisual = Data->DragVisualWidget;
	DragDropOperation->Pivot = EDragPivot::CenterCenter;

	ApplyDragVisualisation();

	// === Applying ===

	DragDropOperation->Payload = this;
	OutOperation = DragDropOperation;
}

void UItemSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	ResetDragVisualisation();
}

bool UItemSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const bool DefaultResult = Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	// === We handle this event only for other slots. So reset drag visual style on dropped widget ===

	UItemSlotWidget* DroppedSlotWidget = Cast<UItemSlotWidget>(InOperation->Payload);

	if (!IsValid(DroppedSlotWidget))
	{
		return DefaultResult;
	}

	DroppedSlotWidget->ResetDragVisualisation();

	// Ignore drop by itself
	if (DroppedSlotWidget == this)
	{
		return DefaultResult;
	}

	// === Determine if a character has the ability to transfer items ===

	const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return DefaultResult;
	}

	const UInventoryManagerComponent* InventoryManagerComponent = Character->GetInventoryManagerComponent();

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

	// === Preparing data for transfer ===

	const FInventorySlot* FromAssociatedInventorySlot = DroppedSlotWidget->GetAssociatedInventorySlot();

#if DO_CHECK
	check(FromAssociatedInventorySlot);
	check(AssociatedInventorySlot);
#endif

	const FInventorySlotsArray* FromInventorySlotsArray = FromAssociatedInventorySlot->GetInventorySlotsArray();
	const FInventorySlotsArray* ToInventorySlotsArray = AssociatedInventorySlot->GetInventorySlotsArray();

#if DO_CHECK
	check(FromInventorySlotsArray);
	check(ToInventorySlotsArray);
#endif

	const FInventorySlotsTypedArray* FromInventorySlotsTypedArray =
		FromInventorySlotsArray->GetInventorySlotsTypedArray();

	const FInventorySlotsTypedArray* ToInventorySlotsTypedArray =
		ToInventorySlotsArray->GetInventorySlotsTypedArray();

#if DO_CHECK
	check(FromInventorySlotsTypedArray);
	check(ToInventorySlotsTypedArray);
#endif

	const FInventorySlotsTypedArrayContainer* FromInventorySlotsTypedArrayContainer =
		FromInventorySlotsTypedArray->GetInventorySlotsTypedArrayContainer();

	const FInventorySlotsTypedArrayContainer* ToInventorySlotsTypedArrayContainer =
		ToInventorySlotsTypedArray->GetInventorySlotsTypedArrayContainer();

#if DO_CHECK
	check(FromInventorySlotsTypedArrayContainer);
	check(ToInventorySlotsTypedArrayContainer);
#endif

	UInventoryManagerComponent* FromInventoryManagerComponent =
		FromInventorySlotsTypedArrayContainer->GetInventoryManagerComponent();

	UInventoryManagerComponent* ToInventoryManagerComponent =
		ToInventorySlotsTypedArrayContainer->GetInventoryManagerComponent();

#if DO_CHECK
	check(FromInventoryManagerComponent);
	check(ToInventoryManagerComponent);
#endif

	// === Do transfer ===

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
