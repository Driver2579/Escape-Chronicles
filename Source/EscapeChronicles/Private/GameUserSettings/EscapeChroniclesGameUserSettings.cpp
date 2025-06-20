// Vladislav Semchuk, 2025

#include "GameUserSettings/EscapeChroniclesGameUserSettings.h"

void UEscapeChroniclesGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	ApplySoundSettings();
}

void UEscapeChroniclesGameUserSettings::ApplySoundSettings()
{
	OnApplySoundSettings.Broadcast();
}

void UEscapeChroniclesGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	MasterVolume = 1;
	MusicVolume = 1;
	SFXVolume = 1;
}