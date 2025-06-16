// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "GameplayTagContainer.h"
#include "MainMenuWidget.generated.h"

class UEscapeChroniclesGameInstance;
class UTextButtonBaseWidget;

UCLASS()
class ESCAPECHRONICLES_API UMainMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> CreateGameButton;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> SettingsButton;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> ExitButton;

	UPROPERTY(EditDefaultsOnly, Category="Sessions")
	TSoftObjectPtr<UWorld> GameLevel;

	void OnCreateGameButtonClicked() const;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful) const;
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful) const;

	// Gameplay tag of the settings menu's route
	UPROPERTY(EditDefaultsOnly, Category="Routes")
	FGameplayTag SettingsMenuRouteTag;

	void OnSettingsButtonClicked() const;

	// A widget that will be shown when the user clicks the exit button
	UPROPERTY(EditDefaultsOnly, Category="Exit")  
	TSubclassOf<class UConfirmationPopup> ExitConfirmationWidgetClass;

	// Text that will be displayed in the exit confirmation widget
	UPROPERTY(EditDefaultsOnly, Category="Exit")  
	FText ExitConfirmationWidgetText;

	void OnExitButtonClicked();
};