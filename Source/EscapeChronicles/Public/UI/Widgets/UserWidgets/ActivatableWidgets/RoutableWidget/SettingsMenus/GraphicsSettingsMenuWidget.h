// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "GraphicsSettingsMenuWidget.generated.h"

class UComboBoxStringWidgetContainer;
class UEscapeChroniclesGameUserSettings;

enum class EGraphicsQuality : uint8;

UCLASS()
class ESCAPECHRONICLES_API UGraphicsSettingsMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UEscapeChroniclesGameUserSettings* GetGameUserSettings() const;

private:
	mutable TWeakObjectPtr<UEscapeChroniclesGameUserSettings> CachedGameUserSettings;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> WindowModeComboBox;

	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TMap<TEnumAsByte<EWindowMode::Type>, FText> WindowModeOptions;

	void InitializeWindowModeComboBox() const;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> ResolutionComboBox;

	void InitializeResolutionComboBox();

	TArray<FIntPoint> Resolutions;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> VSyncComboBox;

	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TMap<bool, FText> VSyncOptions;

	void InitializeVSyncComboBox() const;

	// Options from this map are going to be used for all graphics quality levels that support this format
	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TMap<EGraphicsQuality, FText> SharedGraphicsQualityOptions;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> ViewDistanceQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> ShadowQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> GlobalIlluminationQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> ReflectionQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> AntiAliasingQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> TextureQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> VisualEffectQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> PostProcessingQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> FoliageQualityComboBox;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UComboBoxStringWidgetContainer> ShadingQualityComboBox;

	void InitializeQualityComboBoxes() const;

	UFUNCTION()
	void OnApplyButtonClicked() const;

	// Finds the EGraphicsQuality value that matches the selected graphics quality option and returns it
	EGraphicsQuality GetGraphicsQualityValueFromComboBoxOption(const FString& ComboBoxOption) const;

	int32 GetGraphicsQualityIntValueFromComboBoxOption(const FString& ComboBoxOption) const
	{
		return static_cast<int32>(GetGraphicsQualityValueFromComboBoxOption(ComboBoxOption));
	}
};