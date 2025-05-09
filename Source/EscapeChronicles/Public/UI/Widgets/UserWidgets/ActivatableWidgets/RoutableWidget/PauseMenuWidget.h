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
	UPROPERTY(EditDefaultsOnly, Category="Exiting")  
	FText ExitConfirmationWidgetText;
	
	UPROPERTY(EditDefaultsOnly, Category="Exiting")  
	TSubclassOf<class UConfirmationPopup> ExitConfirmationWidgetClass;

	// Button to resume gameplay 
	UPROPERTY(meta = (BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> ContinueButton;

	// Button to open options submenu 
	UPROPERTY(meta = (BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> OptionsButton;

	// Button to initiate exit flow 
	UPROPERTY(meta = (BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> ExitButton;

	UFUNCTION()
	void OnContinueButtonClicked();

	UFUNCTION()
	void OnOptionsButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();
};
