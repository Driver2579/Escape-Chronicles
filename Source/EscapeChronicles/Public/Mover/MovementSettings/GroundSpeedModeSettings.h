// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MovementMode.h"
#include "GroundSpeedModeSettings.generated.h"

enum class EGroundSpeedMode : uint8;

UCLASS()
class ESCAPECHRONICLES_API UGroundSpeedModeSettings : public UObject, public IMovementSettingsInterface
{
	GENERATED_BODY()

public:
	UGroundSpeedModeSettings();

	virtual void PostReinitProperties() override;

	virtual FString GetDisplayName() const override { return GetName(); }

	EGroundSpeedMode GetDefaultGroundSpeedMode() const
	{
		return DefaultGroundSpeedMode;
	}

	float GetDefaultMaxSpeed() const { return GetMaxSpeedForGroundSpeedMode(DefaultGroundSpeedMode); }

	EGroundSpeedMode GroundSpeedMode;

	// Maximum speed in the movement plane when Walking mode is active
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0, ForceUnits="cm/s"))
	float WalkingMaxSpeed = 300;

	// Maximum speed in the movement plane when Jogging mode is active
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0, ForceUnits="cm/s"))
	float JoggingMaxSpeed = 600;

	// Maximum speed in the movement plane when Running mode is active
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, UIMin=0, ForceUnits="cm/s"))
	float RunningMaxSpeed = 1200;

	float GetSelectedMaxSpeed() const { return GetMaxSpeedForGroundSpeedMode(GroundSpeedMode); }

protected:
	UPROPERTY(EditAnywhere)
	EGroundSpeedMode DefaultGroundSpeedMode;

private:
	float GetMaxSpeedForGroundSpeedMode(const EGroundSpeedMode Mode) const;
};