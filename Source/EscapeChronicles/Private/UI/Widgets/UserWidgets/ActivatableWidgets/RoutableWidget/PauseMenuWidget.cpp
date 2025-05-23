// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/PauseMenuWidget.h"

#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"
#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/ConfirmationPopup.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ensureAlways(IsValid(ContinueButton)))
	{
		ContinueButton->OnClicked().AddUObject(this, &UPauseMenuWidget::OnContinueButtonClicked);
	}

	if (ensureAlways(IsValid(OptionsButton)))
	{
		OptionsButton->OnClicked().AddUObject(this, &UPauseMenuWidget::OnOptionsButtonClicked);
	}

	if (ensureAlways(IsValid(ExitButton)))
	{
		ExitButton->OnClicked().AddUObject(this, &UPauseMenuWidget::OnExitButtonClicked);
	}
}

void UPauseMenuWidget::OnContinueButtonClicked()
{
	DeactivateWidget();
}

void UPauseMenuWidget::OnOptionsButtonClicked()
{
	// TODO: Make the implementation
	unimplemented();
}

void UPauseMenuWidget::OnExitButtonClicked()
{
	UConfirmationPopup* ConfirmationExitWidget = PushPrompt<UConfirmationPopup>(ExitConfirmationWidgetClass);

	if (!ensureAlways(IsValid(ConfirmationExitWidget)))
	{
		return;
	}

	ConfirmationExitWidget->SetDisplayedText(ExitConfirmationWidgetText);
	ConfirmationExitWidget->OnResult().AddLambda([this] (bool bConfirmed)
	{
		if (bConfirmed)
		{
			// TODO: Make an exit to the main menu
			UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit,
				false);
		}
	});
}
