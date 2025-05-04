// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/ActivatableWidgets/YesNoPopup.h"

#include "Components/TextBlock.h"
#include "UI/Widgets/TextButtonBaseWidget.h"

void UYesNoPopup::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (ensureAlways(IsValid(ConfirmButton)))
	{
		ConfirmButton->OnClicked().AddUObject(this, &UYesNoPopup::OnConfirmButtonClicked);
	}

	if (ensureAlways(IsValid(CancelButton)))
	{
		CancelButton->OnClicked().AddUObject(this, &UYesNoPopup::OnCancelButtonClicked);
	}
}

void UYesNoPopup::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	if (IsValid(DisplayedText))
	{
		DisplayedText->SetText(TitleText);
	}
}

void UYesNoPopup::OnConfirmButtonClicked()
{
	ResultEvent.Broadcast(true);
	DeactivateWidget();
}

void UYesNoPopup::OnCancelButtonClicked()
{
	ResultEvent.Broadcast(false);
	DeactivateWidget();
}
