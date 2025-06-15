// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/SaveData/InventorySlotsSaveData.h"
#include "EscapeChroniclesInventoryManagerComponent.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesInventoryManagerComponent : public UInventoryManagerComponent,
	public ISaveable
{
	GENERATED_BODY()

public:
	bool WasEverSaved() const { return bWasEverSaved; }

	virtual void OnPreSaveObject() override;
	virtual void OnPostLoadObject() override;

private:
	// Whether the inventory was ever saved
	UPROPERTY(Transient, SaveGame)
	bool bWasEverSaved = false;

	UPROPERTY(Transient, SaveGame)
	FInventorySlotsSaveData SavedSlotsContainer;
};