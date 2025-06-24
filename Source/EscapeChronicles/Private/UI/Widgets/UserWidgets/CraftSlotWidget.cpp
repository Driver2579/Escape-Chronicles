// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/CraftSlotWidget.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemFragments/IconInventoryItemFragment.h"
#include "Objects/InventoryManagerFragments/InventoryManagerCraftItemsFragment.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/ConfirmationPopup.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/InformPopup.h"

void UCraftSlotWidget::SetAssociate(const FName& InAssociatedRowName,
	const FInventoryManagerCraftData& InAssociatedRowData)
{
	AssociatedRowName = &InAssociatedRowName;
	AssociatedRowData = &InAssociatedRowData;
}

void UCraftSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Data->ToolTipWidget)
	{
		SetToolTip(Data->ToolTipWidget);
	}
}

void UCraftSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!AssociatedRowData || !ensureAlways(AssociatedRowData->ResultItemDefinition))
	{
		return;
	}

	const UInventoryItemDefinition* DefinitionCDO = AssociatedRowData->ResultItemDefinition.GetDefaultObject();

	for (UInventoryItemFragment* Fragment : DefinitionCDO->GetFragments())
	{
		const UIconInventoryItemFragment* IconFragment = Cast<UIconInventoryItemFragment>(Fragment);

		if (IsValid(IconFragment))
		{
			ItemInstanceIcon->SetBrush(IconFragment->GetIcon());

			return;
		}
	}

	ItemInstanceIcon->SetBrush(Data->InvalidItemInstanceBrush);
}

void UCraftSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	UHintBaseWidget* ToolTip = Cast<UHintBaseWidget>(GetToolTip());

	if (!IsValid(ToolTip))
	{
		return;
	}
	
	FText TitleText =
		AssociatedRowData->ResultItemDefinition->GetDefaultObject<UInventoryItemDefinition>()->GetName();

	if (AssociatedRowData->ResultNumber > 1)
	{
		TitleText = FText::Format(FText::FromString("{0} ({1})"), TitleText, AssociatedRowData->ResultNumber);
	}

	FText ConsumableText;
	FText NonConsumableText;

	for (const FCraftRequirement& Material : AssociatedRowData->ItemMaterials)
	{
		FText MaterialText = Material.ItemDefinitionClass->GetDefaultObject<UInventoryItemDefinition>()->GetName();

		if (Material.Number > 1)
		{
			MaterialText = FText::Format(FText::FromString("{0} x{1}"), MaterialText, Material.Number);
		}

		if (Material.bConsumeResource)
		{
			ConsumableText = ConsumableText.IsEmpty() ? MaterialText :
				FText::Format(FText::FromString("{0}, {1}"), ConsumableText, MaterialText);
		}
		else
		{
			NonConsumableText = NonConsumableText.IsEmpty() ? MaterialText :
				FText::Format(FText::FromString("{0}, {1}"), NonConsumableText, MaterialText);
		}
	}

	ToolTip->SetTitleText(TitleText);

	if (NonConsumableText.IsEmpty())
	{
		ToolTip->SetMainText(ConsumableText);
	}
	else
	{
		ToolTip->SetMainText(FText::Format(FText::FromString("{0}\nNot Consumed: {1}"), ConsumableText,
			NonConsumableText));
	}
}

void UCraftSlotWidget::NativeOnClicked()
{
	Super::NativeOnClicked();

	URoutableWidget* RoutableParent = GetTypedOuter<URoutableWidget>();

	if (!ensureAlways(IsValid(RoutableParent)))
	{
		return;
	}

	const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	UInventoryManagerCraftItemsFragment* CraftItemsFragment =
		Inventory->GetFragmentByClass<UInventoryManagerCraftItemsFragment>();

	if (!ensureAlways(IsValid(CraftItemsFragment)))
	{
		return;
	}

	if (!CraftItemsFragment->IsCraftPossible(GetAssociatedRowName()))
	{
		const UInformPopup* InformPopup = RoutableParent->PushPrompt<UInformPopup>(Data->InformPopupClass);

		if (ensureAlways(IsValid(InformPopup)))
		{
			InformPopup->SetDisplayedText(Data->CraftIsNotPossibleInformText);
		}

		return;
	}
	
	UConfirmationPopup* ConfirmationExitWidget = RoutableParent->PushPrompt<UConfirmationPopup>(Data->ConfirmationPopupClass);

	if (!ensureAlways(IsValid(ConfirmationExitWidget)))
	{
		return;
	}

	ConfirmationExitWidget->SetDisplayedText(Data->CraftConfirmationText);
	ConfirmationExitWidget->OnResult.AddWeakLambda(this, [this, CraftItemsFragment](bool bConfirmed)
	{
		if (bConfirmed) CraftItemsFragment->Server_Craft(GetAssociatedRowName());
	});
}
