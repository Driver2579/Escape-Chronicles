// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "PauseMenuWidget.generated.h"

class UTextButtonBaseWidget;

// 
UCLASS()
class ESCAPECHRONICLES_API UPauseMenuWidget : public UMenuWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	
private:
	UPROPERTY(EditAnywhere)  
	FText ConfirmationExitWidgetText;
	
	UPROPERTY(EditAnywhere)  
	TSubclassOf<class UYesNoPopup> ConfirmationExitWidgetClass;
	
	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* ContinueButton;

	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* OptionsButton;

	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* ExitButton;

	UFUNCTION()
	void OnContinueButtonClicked();

	UFUNCTION()
	void OnOptionsButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();
};
