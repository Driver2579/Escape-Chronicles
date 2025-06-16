// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "GraphicsSettingsMenuWidget.generated.h"

class UCommonRotator;
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
	TObjectPtr<UCommonRotator> WindowModeRotator;

	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TMap<TEnumAsByte<EWindowMode::Type>, FText> WindowModeOptions;

	void InitializeWindowModeRotator() const;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> ResolutionRotator;

	void InitializeResolutionRotator();

	TArray<FIntPoint> Resolutions;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> VSyncRotator;

	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TMap<bool, FText> VSyncOptions;

	void InitializeVSyncRotator() const;

	// Options from this map are going to be used for all graphics quality levels that support this format
	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TMap<EGraphicsQuality, FText> SharedGraphicsQualityOptions;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> ViewDistanceQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> ShadowQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> GlobalIlluminationQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> ReflectionQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> AntiAliasingQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> TextureQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> VisualEffectQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> PostProcessingQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> FoliageQualityRotator;

	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonRotator> ShadingQualityRotator;

	void InitializeQualityRotators() const;

	void OnApplyButtonClicked() const;

	// Finds the EGraphicsQuality value that matches the selected graphics quality option and returns it
	EGraphicsQuality GetGraphicsQualityValueFromComboBoxOption(const FText& ComboBoxOption) const;

	int32 GetGraphicsQualityIntValueFromComboBoxOption(const FText& ComboBoxOption) const
	{
		return static_cast<int32>(GetGraphicsQualityValueFromComboBoxOption(ComboBoxOption));
	}
};