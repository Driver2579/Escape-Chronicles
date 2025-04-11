// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FragmentationFragment.generated.h"

class UFragmentationInstance;
/**
 * A fragment that expands the UFragmentationDefinition of an UFragmentationInstance
 */
UCLASS(EditInlineNew, Abstract)
class FRAGMENTATIONSYSTEM_API UFragmentationFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceInitialized(const UFragmentationInstance* Instance) const {};
};
