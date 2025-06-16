// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "SoundSettingsMenuWidget.generated.h"

class USliderWidgetContainer;

UCLASS()
class ESCAPECHRONICLES_API USoundSettingsMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<USliderWidgetContainer> MasterVolumeSlider;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<USliderWidgetContainer> MusicVolumeSlider;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<USliderWidgetContainer> SFXVolumeSlider;

	void InitializeSlidersValuesFromGameUserSettings() const;

	void OnApplyButtonClicked() const;
};