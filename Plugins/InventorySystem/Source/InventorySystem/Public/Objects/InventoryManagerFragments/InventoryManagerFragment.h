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
	
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override
	{
#if DO_CHECK
		check(IsValid(GetOuter()))
#endif

		return GetOuter()->GetFunctionCallspace(Function, Stack);
	}

	virtual bool CallRemoteFunction(UFunction* Function, void* Params, FOutParmRec* OutParams, FFrame* Stack) override
	{
#if DO_CHECK
		check(!HasAnyFlags(RF_ClassDefaultObject));
#endif

		AActor* OwningActor = GetTypedOuter<AActor>();

#if DO_CHECK
		checkf(IsValid(OwningActor),
		  TEXT("Replicated object %s was created with different outer than actor or component!"), *GetName());
#endif

		UNetDriver* NetDriver = OwningActor->GetNetDriver();

		if (IsValid(NetDriver) )
		{
			NetDriver->ProcessRemoteFunction(OwningActor, Function, Params, OutParams, Stack, this);

			return true;
		}

		return false;
	}
};