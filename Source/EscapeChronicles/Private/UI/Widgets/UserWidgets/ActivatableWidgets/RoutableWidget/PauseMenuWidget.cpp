// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/PauseMenuWidget.h"

#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"
#include "EscapeChronicles/Public/UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/ConfirmationPopup.h"
#include "GameInstances/EscapeChroniclesGameInstance.h"
#include "HUDs/EscapeChroniclesHUD.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ensureAlways(ContinueButton))
	{
		ContinueButton->OnClicked().AddUObject(this, &ThisClass::OnContinueButtonClicked);
	}

	if (ensureAlways(SettingsButton))
	{
		SettingsButton->OnClicked().AddUObject(this, &ThisClass::OnSettingsButtonClicked);
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

void UPauseMenuWidget::OnSettingsButtonClicked() const
{
#if DO_CHECK
	check(IsValid(GetOwningPlayer()));
#endif

	AEscapeChroniclesHUD* HUD = GetOwningPlayer()->GetHUD<AEscapeChroniclesHUD>();

	if (ensureAlways(IsValid(HUD)))
	{
#if DO_ENSURE
		ensureAlways(SettingsMenuRouteTag.IsValid());
#endif

		HUD->GoTo(SettingsMenuRouteTag);
	}
}

void UPauseMenuWidget::OnExitButtonClicked()
{
	UConfirmationPopup* ConfirmationExitWidget = PushPrompt<UConfirmationPopup>(ExitConfirmationWidgetClass);

	if (!ensureAlways(IsValid(ConfirmationExitWidget)))
	{
		return;
	}

	ConfirmationExitWidget->SetDisplayedText(ExitConfirmationWidgetText);

	// Close the session and travel to the main menu if the user confirms the exit
	ConfirmationExitWidget->OnResult.AddWeakLambda(this, [this](bool bConfirmed)
	{
		if (!bConfirmed)
		{
			return;
		}

		const UWorld* World = GetWorld();

		UEscapeChroniclesGameInstance* GameInstance = World->GetGameInstanceChecked<UEscapeChroniclesGameInstance>();

		// If the world is a server, destroy the session and travel to the main menu
		if (World->GetNetMode() < NM_Client)
		{
			GameInstance->DestroyHostSession(FOnDestroySessionCompleteDelegate(), true);
		}
		// Otherwise, use the client travel to the main menu
		else
		{
			GameInstance->ClientTravelToMainMenu(World->GetFirstPlayerController());
		}
	});
}