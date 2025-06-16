// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "SettingsMenuWidget.generated.h"

class USoundSettingsMenuWidget;
class UCommonActivatableWidgetSwitcher;
class UGraphicsSettingsMenuWidget;
class UTextButtonBaseWidget;

UCLASS()
class ESCAPECHRONICLES_API USettingsMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

public:
	FSimpleMulticastDelegate OnApply;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> OpenGraphicsSettingsButton;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> OpenSoundSettingsButton;

	void OnGraphicsSettingsMenuBackButtonClicked();
	void OnSoundSettingsMenuBackButtonClicked();

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonActivatableWidgetSwitcher> SettingsWidgetsSwitcher;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGraphicsSettingsMenuWidget> GraphicsSettingsMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USoundSettingsMenuWidget> SoundSettingsMenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UGraphicsSettingsMenuWidget> GraphicsSettingsMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<USoundSettingsMenuWidget> SoundSettingsMenuWidget;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> ApplyButton;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> BackButton;

	void OnApplyButtonClicked();
	void OnBackButtonClicked();
};