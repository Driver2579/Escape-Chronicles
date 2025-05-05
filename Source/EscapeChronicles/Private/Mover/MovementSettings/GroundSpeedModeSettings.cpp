// Fill out your copyright notice in the Description page of Project Settings.

#include "Mover/MovementSettings/GroundSpeedModeSettings.h"

#include "Common/Enums/Mover/GroundSpeedMode.h"

UGroundSpeedModeSettings::UGroundSpeedModeSettings()
	: GroundSpeedMode(EGroundSpeedMode::None)
	, DefaultGroundSpeedMode(EGroundSpeedMode::Jogging)
{
}

void UGroundSpeedModeSettings::PostReinitProperties()
{
	Super::PostReinitProperties();

	GroundSpeedMode = DefaultGroundSpeedMode;
}

float UGroundSpeedModeSettings::GetMaxSpeedForGroundSpeedMode(const EGroundSpeedMode Mode) const
{
	switch (Mode)
	{
	case EGroundSpeedMode::Walking:
		return WalkingMaxSpeed;

	case EGroundSpeedMode::Jogging:
		return JoggingMaxSpeed;

	case EGroundSpeedMode::Running:
		return RunningMaxSpeed;

	default:
		return -1;
	}
}