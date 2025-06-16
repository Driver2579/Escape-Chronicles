// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Saveable.h"
#include "Engine/StreamableManager.h"
#include "BotSpawnerComponent.generated.h"

class UBotNames;
class AEscapeChroniclesAIController;
class AEscapeChroniclesCharacter;

struct FStreamableHandle;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UBotSpawnerComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBotSpawnerComponent();

	/**
	 * Spawns the specified number of each bot type specified in the class properties. Their classes will be loaded
	 * asynchronously before spawning.
	 */
	void SpawnBots();

	// Calls SpawnOneBot the specified number of times with a specified class
	void SpawnBots(const uint8 NumberOfBotsToSpawn,
		const TSubclassOf<AEscapeChroniclesCharacter>& CharacterClass) const;

	/**
	 * Spawns a specified character and restarts it at PlayerStart. Once the AI controller of the character is set, the
	 * USaveGameSubsystem::LoadBotOrGenerateUniquePlayerID function will be called for this bot.
	 * @remark The character must contain the AIController class that is a subclass of AEscapeChroniclesAIController.
	 */
	void SpawnOneBot(const TSubclassOf<AEscapeChroniclesCharacter>& CharacterClass) const;

private:
	// The class of the character to spawn for prisoner bots
	UPROPERTY(EditAnywhere, Category="Prisoners")
	TSoftClassPtr<AEscapeChroniclesCharacter> PrisonerCharacterClass;

	// The number of prisoner bots to spawn when the SpawnBots function is called
	UPROPERTY(EditAnywhere, Category="Prisoners")
	uint8 PrisonerBotsCount = 0;

	// The class of the character to spawn for guard bots
	UPROPERTY(EditAnywhere, Category="Guards")
	TSoftClassPtr<AEscapeChroniclesCharacter> GuardCharacterClass;

	// The number of guard bots to spawn when the SpawnBots function is called
	UPROPERTY(EditAnywhere, Category="Guards")
	uint8 GuardBotsCount = 0;

	// The class of the character to spawn for medic bots
	UPROPERTY(EditAnywhere, Category="Medics")
	TSoftClassPtr<AEscapeChroniclesCharacter> MedicCharacterClass;

	// The number of medic bots to spawn when the SpawnBots function is called
	UPROPERTY(EditAnywhere, Category="Medics")
	uint8 MedicBotsCount = 0;

	// Spawns the prisoner bots and releases the handle
	void OnPrisonerCharacterClassLoaded(TSharedPtr<FStreamableHandle> LoadPrisonerClassesHandle) const;

	// Spawns the guard bots and releases the handle
	void OnGuardCharacterClassLoaded(TSharedPtr<FStreamableHandle> LoadGuardClassesHandle) const;

	// Spawns the medic bots and releases the handle
	void OnMedicCharacterClassLoaded(TSharedPtr<FStreamableHandle> LoadMedicClassesHandle) const;

	UFUNCTION()
	void OnCharacterControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void OnControllerPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/**
	 * A data asset that contains the names that can be assigned to the bots. The name will be selected randomly from
	 * those that aren't claimed yet. If all names are claimed, any random name will be selected.
	 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBotNames> BotNamesDataAsset;

	UPROPERTY(Transient, SaveGame)
	TSet<FString> ClaimedBotNames;

	// Selects a random unclaimed name for the bot. If all names are claimed, any random name will be selected.
	void SelectBotName(FString& OutName);
};