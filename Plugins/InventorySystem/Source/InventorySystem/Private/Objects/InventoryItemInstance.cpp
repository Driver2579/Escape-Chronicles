// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemDefinition);
}
