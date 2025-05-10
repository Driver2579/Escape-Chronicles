// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/DayNightCycleManagerComponent.h"

#include "Engine/DirectionalLight.h"
#include "GameState/EscapeChroniclesGameState.h"

UDayNightCycleManagerComponent::UDayNightCycleManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDayNightCycleManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Don't do anything and disable the tick if no SunLight is set
	if (!ensureAlways(SunLight))
	{
		SetComponentTickEnabledAsync(false);

		return;
	}

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	// Don't do anything and disable the tick if we don't have the required GameState
	if (!ensureAlways(IsValid(GameState)))
	{
		SetComponentTickEnabledAsync(false);

		return;
	}

	// Remember the initial rotation of the sun to rotate only pitch
	InitialSunRotation = SunLight->GetActorRotation();

	// Calculate alphas for sunrise and sunset time of the day (from 0:00-23:59 to 0-1)
	SunriseDayTimeAlpha = static_cast<float>(SunriseTime.ToTotalMinutes()) / FGameplayTime::MaxMinutesInDay;
	SunsetDayTimeAlpha = static_cast<float>(SunsetTime.ToTotalMinutes()) / FGameplayTime::MaxMinutesInDay;

	GameTimeTickPeriod = GameState->GetGameTimeTickPeriod();

	// Remember the current time before it was updated for the first time
	SetCurrentTimeAndPredictNextOne(GameState->GetCurrentGameDateTime().Time);

	// Force set the sun rotation for the first time
	UpdateSunPitchRotation(0);

	GameState->OnCurrentDateTimeUpdated.AddUObject(this, &ThisClass::OnCurrentDateTimeUpdated);
}

void UDayNightCycleManagerComponent::OnCurrentDateTimeUpdated(const FGameplayDateTime& OldDateTime,
	const FGameplayDateTime& NewDateTime)
{
	SetCurrentTimeAndPredictNextOne(NewDateTime.Time);

	// Reset the delta seconds since the last time update because we need to interpolate between the pre
	DeltaSecondsSinceLastTimeUpdate = 0;
}

void UDayNightCycleManagerComponent::SetCurrentTimeAndPredictNextOne(const FGameplayTime& NewTime)
{
	// Remember the new time
	CurrentTime = NewTime;

	// Predict the next time by adding 1 minute to the current time
	PredictedNextTime = CurrentTime;
	++PredictedNextTime;
}

void UDayNightCycleManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSunPitchRotation(DeltaSecondsSinceLastTimeUpdate);

	// Update the seconds since last time update (this is going to be reset in the OnCurrentDateTimeUpdated)
	DeltaSecondsSinceLastTimeUpdate += DeltaTime;
}

void UDayNightCycleManagerComponent::UpdateSunPitchRotation(const float DeltaSecondsBetweenTimeUpdates) const
{
	const float CurrentTimeTotalMinutes = CurrentTime.ToTotalMinutes();
	float PredictedNextTimeTotalMinutes = PredictedNextTime.ToTotalMinutes();

	// Handle midnight crossing for interpolation purposes (e.g., 0:00 -> 24:00, 0:01 -> 24:01)
	if (PredictedNextTimeTotalMinutes < CurrentTimeTotalMinutes)
	{
		PredictedNextTimeTotalMinutes += FGameplayTime::MaxMinutesInDay;
	}

	// Calculate the alpha between the current and the predicted next time
	const float AlphaBetweenCurrentAndNextTime = FMath::Clamp(DeltaSecondsBetweenTimeUpdates / GameTimeTickPeriod,
		0, 1);

	/**
	 * Interpolate between the current and the predicted next time using the calculated alpha (now instead of having 300
	 * and 301 minutes as an example, we have a range between 300 and 301).
	 */
	const float CurrentInterpolatedTotalMinutes = FMath::Lerp(CurrentTimeTotalMinutes, PredictedNextTimeTotalMinutes,
		AlphaBetweenCurrentAndNextTime);

	// Calculate the current alpha of day's time (0 at 0:00, 1 at 24:00) based on the interpolated total minutes
	const float CurrentDayTimeAlpha = FMath::Fmod(CurrentInterpolatedTotalMinutes, FGameplayTime::MaxMinutesInDay) /
		FGameplayTime::MaxMinutesInDay;

	// Define the constants for the sun rotation that are going to be used in the next logic
	constexpr float DayTimeHalfPointAlpha = 0.5; // Sun at 180° mark
	constexpr float SunFullRotationAlpha = 1; // Complete sun rotation (360°)

	// We are going to calculate this in the next logic depending on whether it's day or night
	float SunRotationAngleAlpha;

	// ☀️ Day (from 180° to 360°)
	if (CurrentDayTimeAlpha >= SunriseDayTimeAlpha && CurrentDayTimeAlpha <= SunsetDayTimeAlpha)
	{
		// Calculate how far we are through the day (0 at sunrise, 1 at sunset)
		const float DayAlpha = FMath::GetRangePct(SunriseDayTimeAlpha, SunsetDayTimeAlpha,
			CurrentDayTimeAlpha);
    
		// Map day alpha to second half of rotation (180° -> 360°)
		SunRotationAngleAlpha = FMath::Lerp(DayTimeHalfPointAlpha, SunFullRotationAlpha, DayAlpha);
	}
	// 🌙 Night (from 0° to 180°)
	else
	{
		float NightAlphaUnsafe = CurrentDayTimeAlpha;

		// Add full rotation if after midnight. However, this means that the alpha might be greater than 1.
		if (CurrentDayTimeAlpha < SunriseDayTimeAlpha)
		{
			NightAlphaUnsafe += SunFullRotationAlpha;
		}

		// Calculate the max possible value for unsafe night alpha
		const float NightAlphaUnsafeMaxValue = SunriseDayTimeAlpha + SunFullRotationAlpha;
    
		/**
		 * Normalize the night alpha by changing it from unsafe range to 0-1 range. As a result we get how far we are
		 * through the night (0 at sunset, 1 at sunrise).
		 */
		const float NightAlphaNormalized = FMath::GetRangePct(SunsetDayTimeAlpha, NightAlphaUnsafeMaxValue,
			NightAlphaUnsafe);
    
		// Map night progress to first half of rotation (0° -> 180°)
		SunRotationAngleAlpha = FMath::Lerp(0.f, DayTimeHalfPointAlpha, NightAlphaNormalized);
	}

	// Calculate the sun rotation angle based on the alpha and the full rotation (0° at alpha 0, 360° at alpha 1)
	const float SunRotationAngle = SunRotationAngleAlpha * 360;

#if DO_CHECK
	check(SunLight);
#endif

	// Set the new pitch rotation of the sun
	const FRotator SunRotation = FRotator(SunRotationAngle, InitialSunRotation.Yaw, InitialSunRotation.Roll);
	SunLight->SetActorRotation(SunRotation);
}