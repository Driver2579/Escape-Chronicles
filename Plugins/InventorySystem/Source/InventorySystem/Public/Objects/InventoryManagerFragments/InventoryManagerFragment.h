// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "InventoryManagerFragment.generated.h"

// Base class for creating fragments of an inventory that can describe various logic for it
UCLASS(Abstract, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryManagerFragment : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void OnManagerInitialized(UInventoryManagerComponent* Inventory) {}
	
	UInventoryManagerComponent* GetInventoryManager() const
	{
		UInventoryManagerComponent* Inventory = Cast<UInventoryManagerComponent>(GetOuter());
	
		if (!IsValid(Inventory))
		{
			return nullptr;
		}

		return Inventory;
	}

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Params, FOutParmRec* OutParams, FFrame* Stack) override;
};