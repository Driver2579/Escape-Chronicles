// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SleepingManagementSubsystem.generated.h"

class AEscapeChroniclesCharacter;
class AActivitySpot;
/**
 * Accelerates time if the number of sleeping players is the same as the total number of players
 */
UCLASS(Blueprintable, BlueprintType)
class ESCAPECHRONICLES_API USleepingManagementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	int32 GetSleepingPlayersNumber() const;

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UpdateTimeSpeed() const;
	
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActivitySpot> BedSpotClass;
	
	UPROPERTY(EditAnywhere)
	float SleepTimeDilation = 1;

	float DefaultTimeDilation = 1;

	void OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);
	void OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting);
	
	TArray<TObjectPtr<AActivitySpot>> Beds;

	void OnBedOccupyingCharacterChanged(AEscapeChroniclesCharacter* Character);
};
