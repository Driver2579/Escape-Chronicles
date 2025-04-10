// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FragmentationFragment.h"
#include "Common/Interfaces/Fragmented.h"
#include "FragmentationDefinition.generated.h"

/**
 * Describes an object by storing an array of fragments
 */
UCLASS(Blueprintable, Const, Abstract)
class FRAGMENTATIONSYSTEM_API UFragmentationDefinition : public UObject, public IFragmented
{
	GENERATED_BODY()

public:
	virtual const TArray<TObjectPtr<UFragmentationFragment>>* GetFragments() const override
	PURE_VIRTUAL(UFragmentationDefinition::GetFragments, return nullptr;);
};
