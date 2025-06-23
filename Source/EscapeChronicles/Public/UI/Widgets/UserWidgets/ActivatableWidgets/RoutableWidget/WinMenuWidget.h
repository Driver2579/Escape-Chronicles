// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstances/EscapeChroniclesGameInstance.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"
#include "WinMenuWidget.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UWinMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();

		EndButton->OnClicked().AddWeakLambda(this, [this]
		{
			const UWorld* World = GetWorld();

			UEscapeChroniclesGameInstance* GameInstance =
				World->GetGameInstanceChecked<UEscapeChroniclesGameInstance>();

			if (!ensureAlways(IsValid(GameInstance))) return;

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

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> EndButton;

	
};