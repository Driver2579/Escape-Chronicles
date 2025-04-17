// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/AbilityComponents/GameplayAbilityComponent.h"

bool UGameplayAbilityComponent::CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms,
	FFrame* Stack)
{
#if DO_CHECK
	check(!HasAnyFlags(RF_ClassDefaultObject));
#endif

	AActor* OwningActor = GetTypedOuter<AActor>();

#if DO_CHECK
	check(IsValid(OwningActor));
#endif

	UNetDriver* NetDriver = OwningActor->GetNetDriver();

	if (IsValid(NetDriver))
	{
		NetDriver->ProcessRemoteFunction(OwningActor, Function, Parameters, OutParms, Stack, this);

		return true;
	}

	return false;
}