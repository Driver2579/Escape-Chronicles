// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LootingMenuWidget.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Objects/InventoryManagerFragments/InventoryManagerTransferItemsFragment.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "GameplayMenuWidget.generated.h"

class UInventoryManagerComponent;

// Gameplay menu screen widget
UCLASS()
class ESCAPECHRONICLES_API UGameplayMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override
	{
		
		const AEscapeChroniclesCharacter* Character = GetOwningPlayerPawn<AEscapeChroniclesCharacter>();

		if (!ensureAlways(IsValid(Character)))
		{
			return;
		}

		const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

		if (!ensureAlways(IsValid(Inventory)))
		{
			return;
		}

		UInventoryManagerTransferItemsFragment* InventoryManagerTransferItemsFragment =
			Inventory->GetFragmentByClass<UInventoryManagerTransferItemsFragment>();

		if (!ensureAlways(IsValid(InventoryManagerTransferItemsFragment)))
		{
			return;
		}

		InventoryManagerTransferItemsFragment->OnLootInventoryChanged.AddUObject(this,
			&ThisClass::OnLootInventoryChanged);
	}

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULootingMenuWidget> LootInventoryClass;

	TWeakObjectPtr<ULootingMenuWidget> LootInventory;

	void OnLootInventoryChanged(UInventoryManagerComponent* InInventory)
	{
		if (InInventory != nullptr)
		{
			LootInventory = PushWidget<ULootingMenuWidget>(LootInventoryClass.Get(), true, ERouteInputMode::UI, true);
		}
		else
		{
			LootInventory->DeactivateWidget();
		}
	}
};
