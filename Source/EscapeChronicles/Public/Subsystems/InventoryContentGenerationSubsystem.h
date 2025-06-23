// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Structs/GameplayDateTime.h"
#include "Common/Structs/UniquePlayerID.h"
#include "Interfaces/Saveable.h"
#include "Subsystems/WorldSubsystem.h"
#include "InventoryContentGenerationSubsystem.generated.h"

struct FInventoryContentGeneratingData;
class UInventoryManagerComponent;
struct FStreamableHandle;
class AEscapeChroniclesPlayerState;
class AEscapeChroniclesGameMode;
class AEscapeChroniclesCharacter;
class AActivitySpot;

USTRUCT(BlueprintType)
struct FInventoryContentGenerationEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AEscapeChroniclesCharacter> AvailableOwner;

	UPROPERTY(EditDefaultsOnly)
	int32 AvailableNumber = 1;

	UPROPERTY(EditDefaultsOnly)
	bool bRegenerate = false;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition=bRegenerate))
	FGameplayTime ScheduleRegenerateTime;
};

UCLASS(Blueprintable, BlueprintType)
class ESCAPECHRONICLES_API UInventoryContentGenerationSubsystem : public UWorldSubsystem, public ISaveable
{
	GENERATED_BODY()

public:
	// This system requires settings. Therefore, we allow the creation of only a blueprint version.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/**
	 * Gives items from the table assigned to the character
	 * @param PlayerState Character who needs to clear inventory
	 * @param bClear If I true, first clear the inventory
	 */
	void GenerateInventoryContent(const AEscapeChroniclesPlayerState* PlayerState, bool bClear = true);

private:
	/**
	 * @tparam KeyType Available owners by class
	 * @tparam ValueType DataTable ownership information
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TSoftObjectPtr<UDataTable>, FInventoryContentGenerationEntry> GenerationEntries;

	UPROPERTY(Transient, SaveGame)
	TMap<FUniquePlayerID, TSoftObjectPtr<UDataTable>> DataTableOwnerships;

	int32 GetCurrentDataTableOwnersNumber(const TSoftObjectPtr<UDataTable>& DataTable) const;

	void AssignGeneratingData(const AEscapeChroniclesPlayerState* PlayerState);

	void OnDataTableLoaded(TSharedPtr<FStreamableHandle> Handle, const TSoftObjectPtr<UDataTable> DataTable,
		UInventoryManagerComponent* Inventory);
};
