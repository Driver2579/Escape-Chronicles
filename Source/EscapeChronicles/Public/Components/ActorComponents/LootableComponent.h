// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "LootableComponent.generated.h"

class UInventoryManagerTransferItemsFragment;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPECHRONICLES_API ULootableComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	virtual void OnOpenInventory(UInteractionManagerComponent* InteractionManagerComponent);

private:
	UPROPERTY(EditDefaultsOnly)
	float MaxLootingDistance = 500;

	TArray<TWeakObjectPtr<UInventoryManagerTransferItemsFragment>> Looters;
};
