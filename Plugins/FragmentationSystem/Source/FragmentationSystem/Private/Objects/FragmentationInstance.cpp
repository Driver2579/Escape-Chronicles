// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/FragmentationInstance.h"

#include "Net/UnrealNetwork.h"

UFragmentationInstance::UFragmentationInstance()
{
	ForEachFragment([this](const UFragmentationFragment* Fragment)
	{
		Fragment->OnInstanceCreated(this);
	});
}

void UFragmentationInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Definition);
	DOREPLIFETIME(ThisClass, LocalData);
}

void UFragmentationInstance::ForEachFragment(const TFunctionRef<void(const UFragmentationFragment*)>& Action) const
{
	const UFragmentationDefinition* DefinitionObject = GetDefinitionObject();

	if (!IsValid(DefinitionObject))
	{
		return;
	}

	DefinitionObject->ForEachFragment([Action](const UFragmentationFragment* Fragment)
	{
		Action(Fragment);
	});
}
