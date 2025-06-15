// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Interfaces/Saveable.h"
#include "EscapeChroniclesInventoryManagerComponent.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesInventoryManagerComponent : public UInventoryManagerComponent,
	public ISaveable
{
	GENERATED_BODY()
};