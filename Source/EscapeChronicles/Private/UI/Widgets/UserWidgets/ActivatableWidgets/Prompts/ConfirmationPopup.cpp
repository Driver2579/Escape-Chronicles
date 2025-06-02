// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/ConfirmationPopup.h"

#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"

void UConfirmationPopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (ensureAlways(ConfirmButton))
	{
		ConfirmButton->OnClicked().AddUObject(this, &ThisClass::OnConfirmButtonClicked);
	}

	if (ensureAlways(CancelButton))
	{
		CancelButton->OnClicked().AddUObject(this, &ThisClass::OnCancelButtonClicked);
	}
}

void UConfirmationPopup::SetDisplayedText(const FText& InTitleText) const
{
	if (ensureAlways(DisplayedText))
	{
		DisplayedText->SetText(InTitleText);
	}
}

void UConfirmationPopup::OnConfirmButtonClicked()
{
	OnResult.Broadcast(true);

	DeactivateWidget();
}

void UConfirmationPopup::OnCancelButtonClicked()
{
	OnResult.Broadcast(false);
	
	DeactivateWidget();
}
