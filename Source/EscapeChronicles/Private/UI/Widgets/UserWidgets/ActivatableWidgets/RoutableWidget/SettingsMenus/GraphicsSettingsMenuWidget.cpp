// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/GraphicsSettingsMenuWidget.h"

#include "CommonRotator.h"
#include "Common/Enums/GraphicsQualityLevel.h"
#include "GameUserSettings/EscapeChroniclesGameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/SettingsMenuWidget.h"

void UGraphicsSettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeWindowModeRotator();
	InitializeResolutionRotator();
	InitializeVSyncRotator();
	InitializeQualityRotators();

	USettingsMenuWidget* SettingsMenuWidget = GetTypedOuter<USettingsMenuWidget>();

	if (IsValid(SettingsMenuWidget))
	{
		SettingsMenuWidget->OnApply.AddUObject(this, &ThisClass::OnApplyButtonClicked);
	}
}

UEscapeChroniclesGameUserSettings* UGraphicsSettingsMenuWidget::GetGameUserSettings() const
{
	if (!CachedGameUserSettings.IsValid())
	{
		CachedGameUserSettings = UEscapeChroniclesGameUserSettings::GetEscapeChroniclesGameUserSettings();
	}

	return CachedGameUserSettings.Get();
}

void UGraphicsSettingsMenuWidget::InitializeWindowModeRotator() const
{
	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	TArray<FText> WindowModeOptionsValues;
    WindowModeOptions.GenerateValueArray(WindowModeOptionsValues);

	// Add all window modes to the rotator
    WindowModeRotator->PopulateTextLabels(WindowModeOptionsValues);

	// Find the current window mode option
	const FText* CurrentWindowModeOption = WindowModeOptions.Find(GameUserSettings->GetFullscreenMode());

	// Set the current window mode as rotator's default value
	if (ensureAlways(CurrentWindowModeOption))
	{
		for (int32 i = 0; i < WindowModeOptionsValues.Num(); ++i)
		{
			if (WindowModeOptionsValues[i].EqualTo(*CurrentWindowModeOption))
			{
				WindowModeRotator->SetSelectedItem(i);

				break;
			}
		}
	}
}

void UGraphicsSettingsMenuWidget::InitializeResolutionRotator()
{
	// Get all supported resolutions
	Resolutions.Empty();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	TArray<FText> ResolutionOptions;
	ResolutionOptions.Reserve(Resolutions.Num());

	// Construct an array of all resolution options
	for (const FIntPoint& Resolution : Resolutions)
	{
		ResolutionOptions.Add(
			FText::FromString(FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y)));
	}

	// Add all resolutions to the rotator
	ResolutionRotator->PopulateTextLabels(ResolutionOptions);

	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	// Find the current resolution index
	const FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
	const int32 CurrentResolutionIndex = Resolutions.IndexOfByKey(CurrentResolution);

	// Set the current resolution as rotator's default value
	if (ensureAlways(Resolutions.IsValidIndex(CurrentResolutionIndex)))
	{
		ResolutionRotator->SetSelectedItem(CurrentResolutionIndex);
	}
}

void UGraphicsSettingsMenuWidget::InitializeVSyncRotator() const
{
	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	TArray<FText> VSyncOptionsValues;
	VSyncOptions.GenerateValueArray(VSyncOptionsValues);

	// Add all VSync modes to the rotator
	VSyncRotator->PopulateTextLabels(VSyncOptionsValues);

	// Find the current VSync mode option
	const FText* CurrentVSyncOption = VSyncOptions.Find(GameUserSettings->IsVSyncEnabled());

	// Set the VSync mode as rotator's default value
	if (ensureAlways(CurrentVSyncOption))
	{
		for (int32 i = 0; i < VSyncOptionsValues.Num(); ++i)
		{
			if (VSyncOptionsValues[i].EqualTo(*CurrentVSyncOption))
			{
				VSyncRotator->SetSelectedItem(i);

				break;
			}
		}
	}
}

void UGraphicsSettingsMenuWidget::InitializeQualityRotators() const
{
	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	TArray<FText> QualityOptionsValues;
	SharedGraphicsQualityOptions.GenerateValueArray(QualityOptionsValues);

	// Add all quality options to the rotators
	ViewDistanceQualityRotator->PopulateTextLabels(QualityOptionsValues);
	ShadowQualityRotator->PopulateTextLabels(QualityOptionsValues);
	GlobalIlluminationQualityRotator->PopulateTextLabels(QualityOptionsValues);
	ReflectionQualityRotator->PopulateTextLabels(QualityOptionsValues);
	AntiAliasingQualityRotator->PopulateTextLabels(QualityOptionsValues);
	TextureQualityRotator->PopulateTextLabels(QualityOptionsValues);
	VisualEffectQualityRotator->PopulateTextLabels(QualityOptionsValues);
	PostProcessingQualityRotator->PopulateTextLabels(QualityOptionsValues);
	FoliageQualityRotator->PopulateTextLabels(QualityOptionsValues);
	ShadingQualityRotator->PopulateTextLabels(QualityOptionsValues);

	// Set all current quality levels as rotators' default values
	ViewDistanceQualityRotator->SetSelectedItem(GameUserSettings->GetViewDistanceQuality());
	ShadowQualityRotator->SetSelectedItem(GameUserSettings->GetShadowQuality());
	GlobalIlluminationQualityRotator->SetSelectedItem(GameUserSettings->GetGlobalIlluminationQuality());
	ReflectionQualityRotator->SetSelectedItem(GameUserSettings->GetReflectionQuality());
	AntiAliasingQualityRotator->SetSelectedItem(GameUserSettings->GetAntiAliasingQuality());
	TextureQualityRotator->SetSelectedItem(GameUserSettings->GetTextureQuality());
	VisualEffectQualityRotator->SetSelectedItem(GameUserSettings->GetVisualEffectQuality());
	PostProcessingQualityRotator->SetSelectedItem(GameUserSettings->GetPostProcessingQuality());
	FoliageQualityRotator->SetSelectedItem(GameUserSettings->GetFoliageQuality());
	ShadingQualityRotator->SetSelectedItem(GameUserSettings->GetShadingQuality());
}

void UGraphicsSettingsMenuWidget::OnApplyButtonClicked() const
{
	UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	const FText SelectedWindowModeOption = WindowModeRotator->GetSelectedText();

	// Find the EWindowMode::Type value that matches the selected window mode option and set it as a window mode
	for (const TTuple<TEnumAsByte<EWindowMode::Type>, FText>& WindowModeOption : WindowModeOptions)
	{
		if (SelectedWindowModeOption.EqualTo(WindowModeOption.Value))
		{
			GameUserSettings->SetFullscreenMode(WindowModeOption.Key);

			break;
		}
	}

	GameUserSettings->SetScreenResolution(Resolutions[ResolutionRotator->GetSelectedIndex()]);

	const FText SelectedVSyncOption = VSyncRotator->GetSelectedText();

	// Find the bool value that matches the selected VSync option and set it as a current VSync state
	for (const TTuple<bool, FText>& VSyncOption : VSyncOptions)
	{
		if (SelectedWindowModeOption.EqualTo(VSyncOption.Value))
		{
			GameUserSettings->SetVSyncEnabled(VSyncOption.Key);

			break;
		}
	}

	GameUserSettings->SetViewDistanceQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ViewDistanceQualityRotator->GetSelectedText()));

	GameUserSettings->SetShadowQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ShadowQualityRotator->GetSelectedText()));

	GameUserSettings->SetGlobalIlluminationQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(
			GlobalIlluminationQualityRotator->GetSelectedText()));

	GameUserSettings->SetReflectionQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ReflectionQualityRotator->GetSelectedText()));

	GameUserSettings->SetAntiAliasingQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(AntiAliasingQualityRotator->GetSelectedText()));

	GameUserSettings->SetTextureQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(TextureQualityRotator->GetSelectedText()));

	GameUserSettings->SetVisualEffectQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(VisualEffectQualityRotator->GetSelectedText()));

	GameUserSettings->SetPostProcessingQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(
			PostProcessingQualityRotator->GetSelectedText()));

	GameUserSettings->SetFoliageQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(FoliageQualityRotator->GetSelectedText()));

	GameUserSettings->SetShadingQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ShadingQualityRotator->GetSelectedText()));

	GameUserSettings->ApplySettings(true);
}

EGraphicsQuality UGraphicsSettingsMenuWidget::GetGraphicsQualityValueFromComboBoxOption(
	const FText& ComboBoxOption) const
{
	for (const TTuple<EGraphicsQuality, FText>& QualityOption : SharedGraphicsQualityOptions)
	{
		if (ComboBoxOption.EqualTo(QualityOption.Value))
		{
			return QualityOption.Key;
		}
	}

	return EGraphicsQuality::None;
}