// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/Structs/GameplayDateTime.h"
#include "Sky.generated.h"

class ADirectionalLight;

/**
 * This actor handles the day-night cycle in the game connected to the game time in the GameState. It rotates the
 * sun's pitch based on the current time and the sunrise/sunset times and contains the moon used for the night light,
 * and that rotates the same as the sun but with a 180° offset for the pitch. You can add more components such as the
 * SkyLightComponent, the SkyAtmosphereComponent, etc. in the blueprint.
 */
UCLASS()
class ESCAPECHRONICLES_API ASky : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASky();

	virtual void OnConstruction(const FTransform& Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnCurrentDateTimeUpdated(const FGameplayDateTime& OldDateTime, const FGameplayDateTime& NewDateTime);

private:
	// The sun we are going to rotate (we rotate only the pitch)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDirectionalLightComponent> SunDirectionalLightComponent;

	/**
	 * The moon that is going to be used for the night light. It always rotates the same as the sun but with a 180°
	 * offset for the pitch.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UDirectionalLightComponent> MoonDirectionalLightComponent;

	// The relative rotation of the sun at the start of the game before any updates
	FRotator InitialSunRelativeRotation = FRotator::ZeroRotator;

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

	// Sets the moon rotation to be the same as the sun but with a 180° offset for the pitch
	void UpdateMoonRotation(const FRotator& SunRelativeRotation) const;
};