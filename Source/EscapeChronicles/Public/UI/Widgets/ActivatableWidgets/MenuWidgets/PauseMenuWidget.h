// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidgetBase.h"
#include "PauseMenuWidget.generated.h"

class UTextButtonBaseWidget;

// 
UCLASS()
class ESCAPECHRONICLES_API UPauseMenuWidget : public UMenuWidgetBase
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* ContinueButton;

	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* OptionsButton;

	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* ExitButton;
};
