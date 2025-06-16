// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "PauseMenuWidget.generated.h"

class UTextButtonBaseWidget;

// Pause menu screen widget
UCLASS()
class ESCAPECHRONICLES_API UPauseMenuWidget : public URoutableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	
private:
	// Button to resume gameplay 
	UPROPERTY(meta=(BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> ContinueButton;

	// Button to open settings submenu 
	UPROPERTY(meta=(BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> SettingsButton;

	// Button to initiate exit flow 
	UPROPERTY(meta=(BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> ExitButton;

	UFUNCTION()
	void OnContinueButtonClicked();

	UFUNCTION()
	void OnOptionsButtonClicked();

	// A widget that will be shown when the user clicks the exit button
	UPROPERTY(EditDefaultsOnly, Category="Exit")  
	TSubclassOf<class UConfirmationPopup> ExitConfirmationWidgetClass;

	// Text that will be displayed in the exit confirmation widget
	UPROPERTY(EditDefaultsOnly, Category="Exit")  
	FText ExitConfirmationWidgetText;

	UFUNCTION()
	void OnExitButtonClicked();
};
