// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryManagerFragment.h"

int32 UInventoryManagerFragment::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
#if DO_CHECK
	check(IsValid(GetOuter()))
#endif

	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UInventoryManagerFragment::CallRemoteFunction(UFunction* Function, void* Params, FOutParmRec* OutParams,
	FFrame* Stack)
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

	if (IsValid(NetDriver))
	{
		NetDriver->ProcessRemoteFunction(OwningActor, Function, Params, OutParams, Stack, this);

		return true;
	}

	return false;
}
