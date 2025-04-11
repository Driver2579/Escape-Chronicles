// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/FragmentationFragment.h"
#include "Fragmented.generated.h"

UINTERFACE(MinimalAPI)
class UFragmented : public UInterface
{
	GENERATED_BODY()
};

/* Actual Interface declaration. */
class FRAGMENTATIONSYSTEM_API IFragmented
{
	GENERATED_BODY()

public:
	virtual const TArray<TObjectPtr<UFragmentationFragment>>* GetFragments() const = 0;

	template<typename T>
	const T* GetFragmentByClass() const
	{
		static_assert(TIsDerivedFrom<T, UFragmentationFragment>::Value,
			"T must be inherited from UFragmentationFragment!");
		
		const TArray<TObjectPtr<UFragmentationFragment>>* Fragments = GetFragments();

		if (Fragments == nullptr)
		{
			return nullptr;
		}
		
		for (UFragmentationFragment* Fragment : *Fragments)
		{
			if (T* CastedFragment = Cast<T>(Fragment); IsValid(CastedFragment))
			{
				return CastedFragment;
			}
		}
		
		return nullptr;
	}

	void ForEachFragment(const TFunctionRef<void(const UFragmentationFragment*)>& Action) const
	{
		const TArray<TObjectPtr<UFragmentationFragment>>* Fragments = GetFragments();

		if (Fragments == nullptr)
		{
			return;
		}
		
		for (const auto Fragment : *Fragments)
		{
			if (!IsValid(Fragment))
			{
				return;
			}
			
			Action(Fragment);
		}
	}
};