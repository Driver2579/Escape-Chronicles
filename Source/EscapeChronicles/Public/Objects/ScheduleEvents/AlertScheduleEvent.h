// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScheduleEvent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffectTypes.h"
#include "Common/Structs/SaveData/AlertScheduleEventSaveData.h"
#include "AlertScheduleEvent.generated.h"

class UAbilitySystemComponent;
class AEscapeChroniclesPlayerState;
class UGameplayEffect;

// TODO: Протестировать
// TODO: Загрузка и сохранение не работают корректно

// TODO: Удалить пустые C++ классы ивентов, переместив их в BP
// TODO: Добавить документацию на все ивенты
// TODO: Реализовать перекличку

/**
 * TODO: Implement me
 */
UCLASS()
class ESCAPECHRONICLES_API UAlertScheduleEvent : public UScheduleEvent
{
	GENERATED_BODY()

public:
	void AddWantedPlayer(AEscapeChroniclesPlayerState* PlayerState);

	void RemoveWantedPlayer(AEscapeChroniclesPlayerState* PlayerState,
		const EAllowShrinking AllowShrinking = EAllowShrinking::Yes, const bool bEndEventIfEmpty = true);

	void SetWantedPlayers(const TSet<AEscapeChroniclesPlayerState*>& NewWantedPlayers);
	void SetWantedPlayers(const TSet<FUniquePlayerID>& NewWantedPlayers);

	// TODO: Как только игрок теряет сознание, убирать у него WantedGameplayEffect

	// Should be used when the game is saved
	FAlertScheduleEventSaveData GetAlertScheduleEventSaveData() const;

	// Should be called when the game is loaded
	void LoadAlertScheduleEventFromSaveData(const FAlertScheduleEventSaveData& SaveData);

protected:
	virtual void OnEventStarted(const bool bStartPaused) override;
	virtual void OnEventEnded(const EScheduleEventEndReason EndReason) override;

	virtual void OnEventPaused() override;
	virtual void OnEventResumed() override;

private:
	// List of all wanted players (they might not be in the game)
	TArray<FUniquePlayerID> WantedPlayers;

	// List of wanted players that are currently in the game
	TArray<TWeakObjectPtr<AEscapeChroniclesPlayerState>> WantedPlayersInGame;

	/**
	 * Gameplay effect that used to mark players that are wanted during the alert. This gameplay effect must add the
	 * Status_Wanted tag to the player. Once the event is finished, it will remove it automatically from all players.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> WantedGameplayEffectClass;

	TSharedPtr<FStreamableHandle> LoadWantedGameplayEffectClassHandle;

	void ApplyWantedGameplayEffectToAllWantedPlayers();
	void RemoveWantedGameplayEffectFromAllWantedPlayers();

	void ApplyWantedGameplayEffect(const AEscapeChroniclesPlayerState* PlayerState);
	void RemoveWantedGameplayEffect(const AEscapeChroniclesPlayerState* PlayerState);

	// Called when wanted gameplay effect is removed from the wanted player by anything else than this event
	void OnWantedGameplayEffectExternallyRemoved(const FGameplayEffectRemovalInfo& GameplayEffectRemovalInfo);

	TMap<TWeakObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> ActiveWantedGameplayEffectHandles;

	FDelegateHandle OnPlayerOrBotInitializedDelegateHandle;
	FDelegateHandle OnPlayerOrBotLogoutDelegateHandle;

	void OnPlayerOrBotInitialized(AEscapeChroniclesPlayerState* PlayerState);
	void OnPlayerOrBotLogout(AEscapeChroniclesPlayerState* PlayerState);
};