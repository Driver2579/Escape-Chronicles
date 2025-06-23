// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "EscapeChroniclesCheatManager.generated.h"

class UInventoryItemDefinition;

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	// Sets the current day and time in the game
	UFUNCTION(Exec)
	void Cheat_SetGameDateTime(const uint64 Day, const uint8 Hour, const uint8 Minute) const;

	// Sets the global time speed
	UFUNCTION(Exec)
	void Cheat_SetTimeDilation(const float TimeSpeed) const;

	/**
	 * Hosts a new session and loads the specified level.
	 * @param LevelPath Level we want to load after the session is created. Should be in the next format:
	 * "/Game/Maps/MyMap.MyMap"
	 */
	UFUNCTION(Exec)
	void HostLevel(const FString& LevelPath) const;

	// Destroys the current server session and travels to the main menu
	UFUNCTION(Exec)
	void EndHosting() const;

	// Overrides the base value of the Health attribute in the UVitalAttributeSet
	UFUNCTION(Exec)
	void Cheat_SetHealthBaseAttributeValue(const float NewBaseValue) const;

	// Overrides the base value of the Energy attribute in the UVitalAttributeSet
	UFUNCTION(Exec)
	void Cheat_SetEnergyBaseAttributeValue(const float NewBaseValue) const;

	// Overrides the base value of the Suspicion attribute in the USharedRelationshipAttributeSet
	UFUNCTION(Exec)
	void Cheat_SetSuspicionBaseAttributeValue(const float NewBaseValue) const;

	// Loads all item classes from the asset registry and logs them to the console
	UFUNCTION(Exec)
	void Cheat_LoadAllItemClasses();

	/**
	 * Equips an item into the first available slot of the player's inventory, if any. You should call
	 * Cheat_LoadAllItemClasses before this function to be sure it will work.
	 */
	UFUNCTION(Exec)
	void Cheat_EquipItem(const TSubclassOf<UInventoryItemDefinition>& ItemClass) const;

private:
	// List of all item definitions that can be used in the game
	UPROPERTY(EditAnywhere)
	TSet<TSoftClassPtr<UInventoryItemDefinition>> ItemDefinitions;

	// Called from the HostLevel when the session is created
	static void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	 * Called from the HostLevel when the session is started and calls ServerTravelByLevelSoftObjectPtr for the given
	 * level.
	 */
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful,
		const TSoftObjectPtr<UWorld> LevelToServerTravel) const;
};