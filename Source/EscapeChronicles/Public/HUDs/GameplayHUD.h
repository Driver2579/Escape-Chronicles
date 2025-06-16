// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesHUD.h"
#include "GameplayHUD.generated.h"

class UInventoryManagerComponent;

UCLASS()
class ESCAPECHRONICLES_API AGameplayHUD : public AEscapeChroniclesHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	void OnLootInventoryChanged(UInventoryManagerComponent* InInventory);
};
