// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/ConfirmationPopup.h"

#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"

void UConfirmationPopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (ensureAlways(IsValid(ConfirmButton)))
	{
		ConfirmButton->OnClicked().AddUObject(this, &UConfirmationPopup::OnConfirmButtonClicked);
	}

	if (ensureAlways(IsValid(CancelButton)))
	{
		CancelButton->OnClicked().AddUObject(this, &UConfirmationPopup::OnCancelButtonClicked);
	}
}

void UConfirmationPopup::SetDisplayedText(const FText& InTitleText) const
{
	if (ensureAlways(IsValid(DisplayedText)))
	{
		DisplayedText->SetText(InTitleText);
	}
}

void UConfirmationPopup::OnConfirmButtonClicked()
{
	ResultDelegate.Broadcast(true);

	DeactivateWidget();
}

void UConfirmationPopup::OnCancelButtonClicked()
{
	ResultDelegate.Broadcast(false);
	
	DeactivateWidget();
}
