// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/PauseMenuWidget.h"

#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"
#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/ConfirmationPopup.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ensureAlways(ContinueButton))
	{
		ContinueButton->OnClicked().AddUObject(this, &ThisClass::OnContinueButtonClicked);
	}

	if (ensureAlways(SettingsButton))
	{
		SettingsButton->OnClicked().AddUObject(this, &ThisClass::OnOptionsButtonClicked);
	}

	if (ensureAlways(ExitButton))
	{
		ExitButton->OnClicked().AddUObject(this, &ThisClass::OnExitButtonClicked);
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
	
	ConfirmationExitWidget->OnResult.AddWeakLambda(this, [this](bool bConfirmed)
	{
		if (!bConfirmed)
		{
			return;
		}
		
		// TODO: Make an exit to the main menu
		APlayerController* OwningPlayerController = GetOwningPlayer();
				
		if (IsValid(OwningPlayerController))
		{
			OwningPlayerController->ConsoleCommand("quit");	
		}
	});
}
