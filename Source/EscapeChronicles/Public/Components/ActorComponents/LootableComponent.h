// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "LootableComponent.generated.h"

struct FStreamableHandle;
class AEscapeChroniclesCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPECHRONICLES_API ULootableComponent : public UInteractableComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void OnOpenInventory(UInteractionManagerComponent* InteractionManagerComponent);
};
