// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/MainMenuWidget.h"

#include "GameFramework/PlayerState.h"
#include "GameInstances/EscapeChroniclesGameInstance.h"
#include "HUDs/EscapeChroniclesHUD.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/Prompts/ConfirmationPopup.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateGameButton->OnClicked().AddUObject(this, &ThisClass::OnCreateGameButtonClicked);
	SettingsButton->OnClicked().AddUObject(this, &ThisClass::OnSettingsButtonClicked);
	ExitButton->OnClicked().AddUObject(this, &ThisClass::OnExitButtonClicked);
}

void UMainMenuWidget::OnCreateGameButtonClicked() const
{
	UEscapeChroniclesGameInstance* GameInstance = GetWorld()->GetGameInstanceChecked<UEscapeChroniclesGameInstance>();

	const APlayerState* PlayerState = GetOwningPlayerState(true);

#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	// Host a new session and travel to the game level once the session is created or failed to be created
	GameInstance->StartHostSession(*PlayerState->GetUniqueId().GetUniqueNetId(),
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete),
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete));
}

void UMainMenuWidget::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) const
{
	// Don't do anything if the session was created successfully. The next code is only needed for playing offline. 
	if (bWasSuccessful)
	{
		return;
	}

	UEscapeChroniclesGameInstance* GameInstance = GetWorld()->GetGameInstanceChecked<UEscapeChroniclesGameInstance>();

	// Travel to the game level offline if we failed to create a session
	const bool bTravelResult = GameInstance->ServerTravelByLevelSoftObjectPtr(GameLevel);

	// Destroy the session if we failed
	if (!ensureAlways(bTravelResult))
	{
		GameInstance->DestroyHostSession();
	}
}

void UMainMenuWidget::OnStartSessionComplete(FName SessionName, bool bWasSuccessful) const
{
	UEscapeChroniclesGameInstance* GameInstance = GetWorld()->GetGameInstanceChecked<UEscapeChroniclesGameInstance>();

	// Travel to the game level after the session was either created or failed to be created
	const bool bTravelResult = GameInstance->ServerTravelByLevelSoftObjectPtr(GameLevel);

	// Destroy the session if we failed
	if (!ensureAlways(bTravelResult))
	{
		GameInstance->DestroyHostSession();
	}
}

void UMainMenuWidget::OnSettingsButtonClicked() const
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

void UMainMenuWidget::OnExitButtonClicked()
{
	UConfirmationPopup* ExitConfirmationWidget = PushPrompt<UConfirmationPopup>(ExitConfirmationWidgetClass);

	if (!ensureAlways(IsValid(ExitConfirmationWidget)))
	{
		return;
	}

	ExitConfirmationWidget->SetDisplayedText(ExitConfirmationWidgetText);

	ExitConfirmationWidget->OnResult.AddWeakLambda(this, [this](bool bConfirmed)
	{
		if (!bConfirmed)
		{
			return;
		}

		APlayerController* OwningPlayerController = GetOwningPlayer();
				
		if (IsValid(OwningPlayerController))
		{
			OwningPlayerController->ConsoleCommand("quit");	
		}
	});
}