// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/GameplayDateTime.h"
#include "Components/ActorComponents/ScheduleEventManagerComponent.h"
#include "EscapeChroniclesGameState.generated.h"

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesGameState : public AGameStateBase, public ISaveable
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	float GetGameTimeTickPeriod() const { return GameTimeTickPeriod; }

	const FGameplayDateTime& GetCurrentGameDateTime() const { return CurrentGameDateTime; }

	// Authority only
	void SetCurrentGameDateTime(const FGameplayDateTime& NewGameDateTime);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentGameDateTimeUpdatedDelegate, const FGameplayDateTime& OldDateTime,
		const FGameplayDateTime& NewDateTime);

	FOnCurrentGameDateTimeUpdatedDelegate OnCurrentGameDateTimeUpdated;

	const FScheduleEventData& GetCurrentScheduledEventData() const { return CurrentScheduledEventData; }
	const FScheduleEventData& GetCurrentActiveEventData() const { return CurrentActiveEventData; }

	/**
	 * This delegate duplicates the same delegate from the ScheduleEventManagerComponent except it's replicated to
	 * clients.
	 */
	FOnEventChangedDelegate OnCurrentScheduledEventChanged;

	/**
	 * This delegate duplicates the same delegate from the ScheduleEventManagerComponent except it's replicated to
	 * clients.
	 */
	FOnEventChangedDelegate OnCurrentActiveEventChanged;

protected:
	// Adds a minute to	the current time
	virtual void TickGameDateTime();

	/**
	 * Called on servers when OnCurrentScheduledEventChanged is called in ScheduleEventManagerComponent. Broadcasts the
	 * same delegate via the NetMulticast.
	 */
	virtual void NotifyCurrentScheduledEventChanged(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData);

	/**
	 * Called on servers when OnCurrentActiveEventChanged is called in ScheduleEventManagerComponent. Broadcasts the
	 * same delegate via the NetMulticast.
	 */
	virtual void NotifyCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData);

	virtual void OnPreLoadObject() override;
	virtual void OnPostLoadObject() override;

private:
	void RegisterScheduleEventManagerData();

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
	void OnRep_CurrentDateTime(const FGameplayDateTime& OldValue);

	// Duplicates the value from the ScheduleEventManagerComponent but replicated to clients
	UPROPERTY(Transient, Replicated)
	FScheduleEventData CurrentScheduledEventData;

	// Duplicates the value from the ScheduleEventManagerComponent but replicated to clients
	UPROPERTY(Transient, Replicated)
	FScheduleEventData CurrentActiveEventData;

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastOnCurrentScheduledEventChangedDelegate(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastOnCurrentActiveEventChangedDelegate(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData);

	TSharedPtr<FGameplayDateTime> GameDateTimeBeforeLoading;
};