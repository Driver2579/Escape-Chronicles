// Vladislav Semchuk, 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "EscapeChroniclesGameUserSettings.generated.h"

// A class to handle settings for this game
UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	static UEscapeChroniclesGameUserSettings* GetEscapeChroniclesGameUserSettings()
	{
		return CastChecked<UEscapeChroniclesGameUserSettings>(GetGameUserSettings());
	}

	float GetMasterVolume() const { return MasterVolume; }
	void SetMasterVolume(const float NewVolume) { MasterVolume = FMath::Max(0.f, NewVolume); }

	float GetMusicVolume() const { return MusicVolume; }
	void SetMusicVolume(const float NewVolume) { MusicVolume = FMath::Max(0.f, NewVolume); }

	float GetSFXVolume() const { return SFXVolume; }
	void SetSFXVolume(const float NewVolume) { SFXVolume = FMath::Max(0.f, NewVolume); }

	virtual void ApplyNonResolutionSettings() override;
	virtual void ApplySoundSettings();

	FSimpleMulticastDelegate OnApplySoundSettings;

	virtual void SetToDefaults() override;

private:
	UPROPERTY(Transient, Config)
	float MasterVolume = 1;

	UPROPERTY(Transient, Config)
	float MusicVolume = 1;

	UPROPERTY(Transient, Config)
	float SFXVolume = 1;
};