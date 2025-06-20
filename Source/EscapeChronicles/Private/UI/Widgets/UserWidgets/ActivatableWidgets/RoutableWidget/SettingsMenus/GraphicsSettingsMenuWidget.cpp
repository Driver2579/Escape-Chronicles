// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/GraphicsSettingsMenuWidget.h"

#include "CommonRotator.h"
#include "Common/Enums/GraphicsQualityLevel.h"
#include "Components/ComboBoxString.h"
#include "GameUserSettings/EscapeChroniclesGameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/SettingsMenuWidget.h"
#include "UI/Widgets/UserWidgets/Containers/ComboBoxStringWidgetContainer.h"

void UGraphicsSettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeWindowModeComboBox();
	InitializeResolutionComboBox();
	InitializeVSyncComboBox();
	InitializeQualityComboBoxes();

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

void UGraphicsSettingsMenuWidget::InitializeWindowModeComboBox() const
{
	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	// Clear the existing options if any
	WindowModeComboBox->GetComboBox()->ClearOptions();

	// Add all window modes to the combo box
	for (const TTuple<TEnumAsByte<EWindowMode::Type>, FText>& WindowModeOption : WindowModeOptions)
	{
		WindowModeComboBox->GetComboBox()->AddOption(WindowModeOption.Value.ToString());
	}

	// Find the current window mode option
	const FText* CurrentWindowModeOption = WindowModeOptions.Find(GameUserSettings->GetFullscreenMode());

	// Set the current window mode as combo box's default value
	if (ensureAlways(CurrentWindowModeOption))
	{
		WindowModeComboBox->GetComboBox()->SetSelectedOption(CurrentWindowModeOption->ToString());
	}
}

void UGraphicsSettingsMenuWidget::InitializeResolutionComboBox()
{
	// Get all supported resolutions
	Resolutions.Empty();
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

	// Clear the existing options if any
	ResolutionComboBox->GetComboBox()->ClearOptions();

	// Add all resolutions to the combo box
	for (const FIntPoint& Resolution : Resolutions)
	{
		FString ResolutionOption = FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
		ResolutionComboBox->GetComboBox()->AddOption(ResolutionOption);
	}

	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	// Find the current resolution index
	const FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
	const int32 CurrentResolutionIndex = Resolutions.IndexOfByKey(CurrentResolution);

	// Set the current resolution as combo box's default value
	if (ensureAlways(Resolutions.IsValidIndex(CurrentResolutionIndex)))
	{
		ResolutionComboBox->GetComboBox()->SetSelectedIndex(CurrentResolutionIndex);
	}
}

void UGraphicsSettingsMenuWidget::InitializeVSyncComboBox() const
{
	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	// Clear the existing options if any
	VSyncComboBox->GetComboBox()->ClearOptions();

	// Add all VSync options to the combo box
	for (const TTuple<bool, FText>& VSyncOption : VSyncOptions)
	{
		VSyncComboBox->GetComboBox()->AddOption(VSyncOption.Value.ToString());
	}

	// Find the VSync mode option
	const FText* CurrentVSyncOption = VSyncOptions.Find(GameUserSettings->IsVSyncEnabled());

	// Set the current VSync mode as combo box's default value
	if (ensureAlways(CurrentVSyncOption))
	{
		VSyncComboBox->GetComboBox()->SetSelectedOption(CurrentVSyncOption->ToString());
	}
}

void UGraphicsSettingsMenuWidget::InitializeQualityComboBoxes() const
{
	const UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	// Clear the existing options if any
	ViewDistanceQualityComboBox->GetComboBox()->ClearOptions();
	ShadowQualityComboBox->GetComboBox()->ClearOptions();
	GlobalIlluminationQualityComboBox->GetComboBox()->ClearOptions();
	ReflectionQualityComboBox->GetComboBox()->ClearOptions();
	AntiAliasingQualityComboBox->GetComboBox()->ClearOptions();
	TextureQualityComboBox->GetComboBox()->ClearOptions();
	VisualEffectQualityComboBox->GetComboBox()->ClearOptions();
	PostProcessingQualityComboBox->GetComboBox()->ClearOptions();
	FoliageQualityComboBox->GetComboBox()->ClearOptions();
	ShadingQualityComboBox->GetComboBox()->ClearOptions();

	// Reserve space for the cached quality options to avoid reallocations
	TArray<FString> CachedQualityOptions;
	CachedQualityOptions.Reserve(static_cast<int32>(EGraphicsQuality::NumQualityLevels));

	// Cache all quality options to a string array to avoid calling FText::ToString() every time we need any
	for (const TTuple<EGraphicsQuality, FText>& QualityOption : SharedGraphicsQualityOptions)
	{
		// Insert the quality option into the index that matches the enum value
		CachedQualityOptions.Insert(QualityOption.Value.ToString(), static_cast<int32>(QualityOption.Key));
	}

	// Add all quality options to the combo boxes
	for (int32 i = 0; i < CachedQualityOptions.Num(); ++i)
	{
		ViewDistanceQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		ShadowQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		GlobalIlluminationQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		ReflectionQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		AntiAliasingQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		TextureQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		VisualEffectQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		PostProcessingQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		FoliageQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
		ShadingQualityComboBox->GetComboBox()->AddOption(CachedQualityOptions[i]);
	}

	// Set all current quality levels as combo boxes' default values
	ViewDistanceQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetViewDistanceQuality());
	ShadowQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetShadowQuality());
	GlobalIlluminationQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetGlobalIlluminationQuality());
	ReflectionQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetReflectionQuality());
	AntiAliasingQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetAntiAliasingQuality());
	TextureQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetTextureQuality());
	VisualEffectQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetVisualEffectQuality());
	PostProcessingQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetPostProcessingQuality());
	FoliageQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetFoliageQuality());
	ShadingQualityComboBox->GetComboBox()->SetSelectedIndex(GameUserSettings->GetShadingQuality());
}

void UGraphicsSettingsMenuWidget::OnApplyButtonClicked() const
{
	UEscapeChroniclesGameUserSettings* GameUserSettings = GetGameUserSettings();

#if DO_CHECK
	check(IsValid(GameUserSettings));
#endif

	const FString SelectedWindowModeOption = WindowModeComboBox->GetComboBox()->GetSelectedOption();

	// Find the EWindowMode::Type value that matches the selected window mode option and set it as a window mode
	for (const TTuple<TEnumAsByte<EWindowMode::Type>, FText>& WindowModeOption : WindowModeOptions)
	{
		if (SelectedWindowModeOption.Equals(WindowModeOption.Value.ToString()))
		{
			GameUserSettings->SetFullscreenMode(WindowModeOption.Key);

			break;
		}
	}

	GameUserSettings->SetScreenResolution(Resolutions[ResolutionComboBox->GetComboBox()->GetSelectedIndex()]);

	const FString SelectedVSyncOption = VSyncComboBox->GetComboBox()->GetSelectedOption();

	// Find the bool value that matches the selected VSync option and set it as a current VSync state
	for (const TTuple<bool, FText>& VSyncOption : VSyncOptions)
	{
		if (SelectedWindowModeOption.Equals(VSyncOption.Value.ToString()))
		{
			GameUserSettings->SetVSyncEnabled(VSyncOption.Key);

			break;
		}
	}

	GameUserSettings->SetViewDistanceQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ViewDistanceQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetShadowQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ShadowQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetGlobalIlluminationQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(
			GlobalIlluminationQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetReflectionQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ReflectionQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetAntiAliasingQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(AntiAliasingQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetTextureQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(TextureQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetVisualEffectQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(VisualEffectQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetPostProcessingQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(
			PostProcessingQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetFoliageQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(FoliageQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->SetShadingQuality(
		GetGraphicsQualityIntValueFromComboBoxOption(ShadingQualityComboBox->GetComboBox()->GetSelectedOption()));

	GameUserSettings->ApplySettings(true);
}

EGraphicsQuality UGraphicsSettingsMenuWidget::GetGraphicsQualityValueFromComboBoxOption(
	const FString& ComboBoxOption) const
{
	for (const TTuple<EGraphicsQuality, FText>& QualityOption : SharedGraphicsQualityOptions)
	{
		if (ComboBoxOption.Equals(QualityOption.Value.ToString()))
		{
			return QualityOption.Key;
		}
	}

	return EGraphicsQuality::None;
}