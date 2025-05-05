// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EGroundSpeedMode : uint8
{
	// This mode should be used for slow movement
	Walking,

	// This mode should be used for medium-speed movement
	Jogging,

	// This mode should be used for fast movement
	Running,

	NumberOfModes UMETA(Hidden),
	None UMETA(Hidden)
};