// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Fragmented.generated.h"

class UFragment;

UINTERFACE(MinimalAPI)
class UFragmented : public UInterface
{
	GENERATED_BODY()
};

class FRAGMENTSSYSTEM_API IFragmented
{
	GENERATED_BODY()

public:
	virtual const TArray<TObjectPtr<UFragment>>& GetFragments() const = 0;

	template<typename T>
	const T* GetFragmentByClass() const
	{
		return GetFragmentByClass_Mutable<T>();
	}
	
	template<typename T>
	T* GetFragmentByClass_Mutable() const
	{
		static_assert(TIsDerivedFrom<T, UFragment>::Value, "T must be inherited from UFragment!");
		
		for (UFragment* Fragment : GetFragments())
		{
			T* CastedFragment = Cast<T>(Fragment);
			
			if (IsValid(CastedFragment))
			{
				return CastedFragment;
			}
		}
		
		return nullptr;
	}

	template<typename T>
	void GetFragmentsByClass(TArray<const T*>& OutFragments) const
	{
		GetFragmentsByClass_Mutable<T>(OutFragments);
	}
	
	template<typename T>
	void GetFragmentsByClass_Mutable(TArray<T*>& OutFragments) const
	{
		static_assert(TIsDerivedFrom<T, UFragment>::Value, "T must be inherited from UFragment!");
		
		for (UFragment* Fragment : GetFragments())
		{
			T* CastedFragment = Cast<T>(Fragment);
			
			if (IsValid(CastedFragment))
			{
				OutFragments.Add(CastedFragment);
			}
		}
	}
};