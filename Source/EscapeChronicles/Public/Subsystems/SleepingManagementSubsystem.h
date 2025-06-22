// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Common/Structs/ScheduleEventData.h"
#include "SleepingManagementSubsystem.generated.h"

class AEscapeChroniclesCharacter;
class AActivitySpot;

// Accelerates time if the number of sleeping players is the same as the total number of players
UCLASS(Blueprintable, BlueprintType)
class ESCAPECHRONICLES_API USleepingManagementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// This system requires settings. Therefore, we allow the creation of only a blueprint version.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// Returns the number of player controllers who are sleeping right now
	int32 GetSleepingPlayersNumber() const;

	// Sets TimeDilation depending on whether players are sleeping
	void UpdateTimeSpeed() const;

protected:
	virtual void OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData);

private:
	// Using these objects will be considered as if the player is sleeping
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActivitySpot> BedClass;

	// TimeDilation when everyone is asleep
	UPROPERTY(EditAnywhere)
	float SleepTimeDilation = 1;

	// If specified, then to change the time dilation, the current active event must have one of these tags
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer RequiredEventsToChangeTimeDilation;

	FGameplayTag CurrentActiveEventTag;

	// Events when TimeDilation is updated
	void OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer) const;
	void OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting) const;
	void OnBedOccupyingCharacterChanged(AEscapeChroniclesCharacter* Character) const;

	/**
	 * All beds sleeping on which sets SleepTimeDilation.
	 * @see This array is initialized only at the start of the game. Actors that are spawned at runtime are not counted.
	 */
	TArray<TObjectPtr<AActivitySpot>> Beds;
};
