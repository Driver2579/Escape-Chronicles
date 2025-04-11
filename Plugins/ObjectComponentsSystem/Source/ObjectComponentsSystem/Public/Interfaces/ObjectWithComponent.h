// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectWithComponent.generated.h"

class UObjectComponent;

UINTERFACE(MinimalAPI)
class UObjectWithComponent : public UInterface
{
	GENERATED_BODY()
};

class OBJECTCOMPONENTSSYSTEM_API IObjectWithComponent
{
	GENERATED_BODY()

public:
	virtual const TArray<TObjectPtr<UObjectComponent>>& GetComponents() const = 0;

	template<typename T>
	const T* GetComponentByClass() const
	{
		return GetComponentByClass_Mutable<T>();
	}
    
	template<typename T>
	T* GetComponentByClass_Mutable() const
	{
		static_assert(TIsDerivedFrom<T, UObjectComponent>::Value, "T must be inherited from UObjectComponent!");
        
		for (UObjectComponent* Component : GetComponents())
		{
			T* CastedComponent = Cast<T>(Component);
            
			if (IsValid(CastedComponent))
			{
				return CastedComponent;
			}
		}
        
		return nullptr;
	}

	template<typename T>
	void GetComponentsByClass(TArray<const T*>& OutComponents) const
	{
		GetComponentsByClass_Mutable<T>(OutComponents);
	}
    
	template<typename T>
	void GetComponentsByClass_Mutable(TArray<T*>& OutComponents) const
	{
		static_assert(TIsDerivedFrom<T, UObjectComponent>::Value, "T must be inherited from UObjectComponent!");
        
		for (UObjectComponent* Component : GetComponents())
		{
			T* CastedComponent = Cast<T>(Component);
            
			if (IsValid(CastedComponent))
			{
				OutComponents.Add(CastedComponent);
			}
		}
	}
};