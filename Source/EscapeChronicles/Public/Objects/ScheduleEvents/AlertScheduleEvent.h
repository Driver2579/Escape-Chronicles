// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScheduleEvent.h"
#include "AlertScheduleEvent.generated.h"

class UGameplayEffect;

/**
 * TODO: Implement me
 */
UCLASS()
class ESCAPECHRONICLES_API UAlertScheduleEvent : public UScheduleEvent
{
	GENERATED_BODY()

	// TODO: Как только игрок теряет сознание, убирать у него WantedGameplayEffect
	// TODO: Как только все игроки с WantedGameplayEffect теряют сознание, заканчивать этот ивент

protected:
	virtual void OnEventEnded() override;

private:
	/**
	 * Gameplay effect that used to mark players that are wanted during the alert. This gameplay effect must add the
	 * Status_Wanted tag to the player, and this gameplay effect must be added manually to wanted players during this
	 * event is active. Once the event is finished, it will remove it automatically from all players.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> WantedGameplayEffectClass;
};