// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BedsManagementSubsystem.generated.h"

/**
 * Accelerates time if the number of sleeping players is the same as the total number of players (works with
 * UBedsManagementSubsystem)
 */
UCLASS(Blueprintable, BlueprintType)
class ESCAPECHRONICLES_API UBedsManagementSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	int32 GetSleepingPlayersNumber() const { return SleepingPlayersNumber; }
	void IncreaseSleepingPlayersNumber();
	void DecreaseSleepingPlayersNumber();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UpdateTimeSpeed() const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	UPROPERTY(EditAnywhere)
	float SleepTimeDilation = 1;

	float DefaultTimeDilation = 1;

	void OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);
	void OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting);

	UPROPERTY(Replicated)
	int32 SleepingPlayersNumber = 0;
};
