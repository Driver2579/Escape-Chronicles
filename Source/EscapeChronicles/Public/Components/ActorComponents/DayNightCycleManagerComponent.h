// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Structs/GameplayDateTime.h"
#include "Components/ActorComponent.h"
#include "DayNightCycleManagerComponent.generated.h"

class ADirectionalLight;

// TODO: Rework this component into a sky actor that contains the sun and the moon

/**
 * This component handles the day-night cycle in the game connected to the game time in the GameState. It rotates the
 * sun's pitch based on the current time and the sunrise/sunset times.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UDayNightCycleManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDayNightCycleManagerComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnCurrentDateTimeUpdated(const FGameplayDateTime& OldDateTime, const FGameplayDateTime& NewDateTime);

private:
	// The sun we are going to rotate (we rotate only the pitch)
	UPROPERTY(EditAnywhere)
	TObjectPtr<ADirectionalLight> SunLight;

	// The rotation of the sun at the start of the game before any updates
	FRotator InitialSunRotation = FRotator::ZeroRotator;

	// The time when the sunrise starts
	UPROPERTY(EditAnywhere)
	FGameplayTime SunriseTime = FGameplayTime(6, 0);

	// The time when the sunset ends
	UPROPERTY(EditAnywhere)
	FGameplayTime SunsetTime = FGameplayTime(20, 0);

	// SunriseTime converted to alpha (0-1, where 0 is 0:00 and 1 is 24:00)
	float SunriseDayTimeAlpha = 0;

	// SunsetTime converted to alpha (0-1, where 0 is 0:00 and 1 is 24:00)
	float SunsetDayTimeAlpha = 0;

	// The value from the GameState
	float GameTimeTickPeriod = 1;

	void SetCurrentTimeAndPredictNextOne(const FGameplayTime& NewTime);

	FGameplayTime CurrentTime;
	FGameplayTime PredictedNextTime;

	/**
	 * The delta seconds that have been passed since the last time the CurrentTime and the PredictedNextTime were
	 * updated.
	 */
	float DeltaSecondsSinceLastTimeUpdate = 0;

	/**
	 * @param DeltaSecondsBetweenTimeUpdates The delta seconds that have been passed since the last time the CurrentTime
	 * and the PredictedNextTime were updated.
	 */
	void UpdateSunPitchRotation(const float DeltaSecondsBetweenTimeUpdates) const;
};