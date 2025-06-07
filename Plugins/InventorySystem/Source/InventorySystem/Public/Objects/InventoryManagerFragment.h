// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "InventoryManagerFragment.generated.h"

// Base class for inventory's fragments that can describe various logic for it
UCLASS(Abstract, EditInlineNew)
class INVENTORYSYSTEM_API UInventoryManagerFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual inline int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Params, FOutParmRec* OutParams, FFrame* Stack) override;

	virtual void OnManagerInitialized() {}

	UInventoryManagerComponent* GetInventoryManager() const
	{
		return GetTypedOuter<UInventoryManagerComponent>();
	}
};