// Vladislav Semchuk, 2025

#pragma once

/**
 * This enum represents the graphics settings for the GameUserSettings class that wait for a 0..4 integer. It can be
 * directly converted to int32 values for such graphics settings in the GameUserSettings class.
 */
UENUM(BlueprintType)
enum class EGraphicsQuality : uint8
{
	Low,
	Medium,
	High,
	Epic,
	Cinematic,

	// The total number of supported quality levels
	NumQualityLevels UMETA(Hidden),

	None = 255 UMETA(Hidden)
};