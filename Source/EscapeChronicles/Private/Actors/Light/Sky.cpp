// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Light/Sky.h"

#include "Components/DirectionalLightComponent.h"
#include "GameState/EscapeChroniclesGameState.h"

ASky::ASky()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SunDirectionalLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	SunDirectionalLightComponent->SetupAttachment(RootComponent);

	MoonDirectionalLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Moon"));
	MoonDirectionalLightComponent->SetupAttachment(RootComponent);

	// Set the moon intensity to be very low
	MoonDirectionalLightComponent->Intensity = 0.2;

	// Set the moon temperature to be twice more as the sun temperature
	MoonDirectionalLightComponent->bUseTemperature = true;
	MoonDirectionalLightComponent->Temperature = SunDirectionalLightComponent->Temperature * 2;

	// Set the moon to be a secondary light source for the atmosphere
	MoonDirectionalLightComponent->AtmosphereSunLightIndex = 1;
	MoonDirectionalLightComponent->ForwardShadingPriority = 1;
}

void ASky::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Update the moon rotation based on the sun rotation
	UpdateMoonRotation(SunDirectionalLightComponent->GetRelativeRotation());
}

void ASky::BeginPlay()
{
	Super::BeginPlay();

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	// Don't do anything and disable the tick if we don't have the required GameState
	if (!ensureAlways(IsValid(GameState)))
	{
		SetActorTickEnabled(false);

		return;
	}

	// Remember the initial rotation of the sun to rotate only pitch
	InitialSunRelativeRotation = SunDirectionalLightComponent->GetRelativeRotation();

	// Calculate alphas for sunrise and sunset time of the day (from 0:00-23:59 to 0-1)
	SunriseDayTimeAlpha = static_cast<float>(SunriseTime.ToTotalMinutes()) / FGameplayTime::MaxMinutesInDay;
	SunsetDayTimeAlpha = static_cast<float>(SunsetTime.ToTotalMinutes()) / FGameplayTime::MaxMinutesInDay;

	GameTimeTickPeriod = GameState->GetGameTimeTickPeriod();

	// Remember the current time before it was updated for the first time
	SetCurrentTimeAndPredictNextOne(GameState->GetCurrentGameDateTime().Time);

	// Force set the sun rotation for the first time
	UpdateSunPitchRotation(0);

	GameState->OnCurrentGameDateTimeUpdated.AddUObject(this, &ThisClass::OnCurrentGameDateTimeUpdated);
}

void ASky::OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime, const FGameplayDateTime& NewDateTime)
{
	SetCurrentTimeAndPredictNextOne(NewDateTime.Time);

	// Reset the delta seconds since the last time update because we need to interpolate between the pre
	DeltaSecondsSinceLastTimeUpdate = 0;
}

void ASky::SetCurrentTimeAndPredictNextOne(const FGameplayTime& NewTime)
{
	// Remember the new time
	CurrentTime = NewTime;

	// Predict the next time by adding 1 minute to the current time
	PredictedNextTime = CurrentTime;
	++PredictedNextTime;
}

void ASky::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSunPitchRotation(DeltaSecondsSinceLastTimeUpdate);

	// Update the seconds since last time update (this is going to be reset in the OnCurrentGameDateTimeUpdated)
	DeltaSecondsSinceLastTimeUpdate += DeltaTime;
}

void ASky::UpdateSunPitchRotation(const float DeltaSecondsBetweenTimeUpdates) const
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

	// Create the rotation for the sun with a new pitch
	const FRotator SunRotation = FRotator(SunRotationAngle, InitialSunRelativeRotation.Yaw,
		InitialSunRelativeRotation.Roll);

	// Set the new rotation for the sun
	SunDirectionalLightComponent->SetRelativeRotation(SunRotation);

	// Update the moon rotation based on the sun rotation
	UpdateMoonRotation(SunRotation);
}

void ASky::UpdateMoonRotation(const FRotator& SunRelativeRotation) const
{
	MoonDirectionalLightComponent->SetRelativeRotation(
		FRotator(SunRelativeRotation.Pitch + 180, SunRelativeRotation.Yaw, SunRelativeRotation.Roll));
}