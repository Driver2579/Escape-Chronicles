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

/**
 * An event that marks specific players as "wanted" with gameplay effect applied. "Wanted player" means that the guards
 * should hunt him. The medics should remove the "wanted" mark once they cared the wanted player to the hospital after
 * he passes out. Once there are no wanted players left, the event will end automatically. At the end of the event, if
 * it was ended normally, all destructed holes in DestructibleComponents that are owned by DynamicMeshActors will be
 * cleared, if any.
 * @remark When this event is started, you MUST call the AddWantedPlayer function or SetWantedPlayers function for each
 * player you want to mark as wanted after the event is started.
 */
UCLASS()
class ESCAPECHRONICLES_API UAlertScheduleEvent : public UScheduleEvent
{
	GENERATED_BODY()

public:
	// Adds the player to the wanted players list
	void AddWantedPlayer(AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * Removes the player from the wanted players list.
	 * @param PlayerState Player to remove from the list. 
	 * @param AllowShrinking Whether the arrays with wanted players should be shrunk or not. If set to No, then you
	 * should shrink them yourself.
	 * @param bEndEventIfEmpty If true, then, if there are no wanted players left in the game, the event will be ended.
	 */
	void RemoveWantedPlayer(AEscapeChroniclesPlayerState* PlayerState,
		const EAllowShrinking AllowShrinking = EAllowShrinking::Yes, const bool bEndEventIfEmpty = true);

	/**
	 * Replaces the wanted players list with the new one. If there are no wanted players left in the game, the event
	 * will be ended.
	 */
	void SetWantedPlayers(const TSet<AEscapeChroniclesPlayerState*>& NewWantedPlayers);
	void SetWantedPlayers(const TSet<FUniquePlayerID>& NewWantedPlayers);

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