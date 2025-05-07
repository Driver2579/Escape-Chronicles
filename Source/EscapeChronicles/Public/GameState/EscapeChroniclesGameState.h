// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Common/Structs/GameplayDateTime.h"
#include "Interfaces/Saveable.h"
#include "EscapeChroniclesGameState.generated.h"

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesGameState : public AGameStateBase, public ISaveable
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	const FGameplayDateTime& GetCurrentGameDateTime() const { return CurrentGameDateTime; }

	// Authority only
	void SetCurrentGameDateTime(const FGameplayDateTime& NewGameTime);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentTimeUpdatedDelegate, const FGameplayDateTime& NewTime);

	FOnCurrentTimeUpdatedDelegate OnCurrentDateTimeUpdated;

protected:
	// Adds a minute to	the current time
	virtual void TickGameDateTime();

	virtual void OnPreLoadObject() override;

private:
	// The time this game starts with
	UPROPERTY(EditDefaultsOnly, Category="Game Time")
	FGameplayDateTime StartGameDateTime;

	UPROPERTY(Transient, SaveGame, ReplicatedUsing="OnRep_CurrentDateTime")
	FGameplayDateTime CurrentGameDateTime;

	// How often the game time should be incremented by one minute
	UPROPERTY(EditDefaultsOnly, Category="Game Time", meta=(ClampMin=0.1))
	float GameTimeTickPeriod = 1;

	FTimerHandle TickGameTimeTimerHandle;

	void RestartTickGameTimeTimer();

	UFUNCTION()
	void OnRep_CurrentDateTime();
};