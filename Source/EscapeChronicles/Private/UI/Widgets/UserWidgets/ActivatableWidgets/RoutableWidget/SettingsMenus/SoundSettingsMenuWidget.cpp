// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/SoundSettingsMenuWidget.h"

#include "Components/Slider.h"
#include "GameUserSettings/EscapeChroniclesGameUserSettings.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/SettingsMenuWidget.h"
#include "UI/Widgets/UserWidgets/Containers/SliderWidgetContainer.h"

void USoundSettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeSlidersValuesFromGameUserSettings();

	USettingsMenuWidget* SettingsMenuWidget = GetTypedOuter<USettingsMenuWidget>();

	if (IsValid(SettingsMenuWidget))
	{
		SettingsMenuWidget->OnApply.AddUObject(this, &ThisClass::OnApplyButtonClicked);
	}
}

void USoundSettingsMenuWidget::InitializeSlidersValuesFromGameUserSettings() const
{
	const UEscapeChroniclesGameUserSettings* GameUserSettings =
		UEscapeChroniclesGameUserSettings::GetEscapeChroniclesGameUserSettings();

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->GetSlider()->SetValue(GameUserSettings->GetMasterVolume());
	}

	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->GetSlider()->SetValue(GameUserSettings->GetMusicVolume());
	}

	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->GetSlider()->SetValue(GameUserSettings->GetSFXVolume());
	}
}

void USoundSettingsMenuWidget::OnApplyButtonClicked() const
{
	UEscapeChroniclesGameUserSettings* GameUserSettings =
		UEscapeChroniclesGameUserSettings::GetEscapeChroniclesGameUserSettings();

	GameUserSettings->SetMasterVolume(MasterVolumeSlider->GetSlider()->GetValue());
	GameUserSettings->SetMusicVolume(MusicVolumeSlider->GetSlider()->GetValue());
	GameUserSettings->SetSFXVolume(SFXVolumeSlider->GetSlider()->GetValue());

	GameUserSettings->ApplySoundSettings();
	GameUserSettings->SaveSettings();
}