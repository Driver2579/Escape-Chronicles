// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FragmentationDefinition.h"
#include "Common/Structs/FastArraySerializers/FragmentationLocalData.h"
#include "FragmentationInstance.generated.h"

/**
 * Represents an instance of an object
 */
UCLASS(EditInlineNew)
class FRAGMENTATIONSYSTEM_API UFragmentationInstance : public UObject
{
	GENERATED_BODY()
	
public:
	FFragmentationLocalData& GetLocalData() 
	{
		return LocalData;
	}
	
	UFragmentationDefinition* GetDefinitionObject() const
	{
		return GetDefinitionObject<UFragmentationDefinition>();
	}
	
	template<typename T>
	T* GetDefinitionObject() const
	{
		static_assert(std::is_base_of_v<UFragmentationDefinition, T>, "T must inherit from UFragmentationDefinition!");
		
		if (!IsValid(Definition))
		{
			return nullptr;
		}
		
		return Definition->GetDefaultObject<T>();
	}

	template<typename T>
	const T* GetFragmentByClass() const
	{
		static_assert(std::is_base_of_v<UFragmentationFragment, T>, "T must inherit from UFragmentationDefinition!");
		
		if (!IsValid(Definition))
		{
			return nullptr;
		}
		
		const UFragmentationDefinition* DefinitionCDO = Definition->GetDefaultObject<UFragmentationDefinition>();

		if (!IsValid(DefinitionCDO))
		{
			return nullptr;
		}

		return DefinitionCDO->GetFragmentByClass<T>();
	}

	void Initialize(const TSubclassOf<UFragmentationDefinition>& InDefinition = nullptr)
	{
		if (!ensureAlwaysMsgf(!bInitialized, TEXT("The instance must only be initialized once!")))
		{
			return;
		}
		
		if (InDefinition != nullptr && ensureAlways(IsValid(InDefinition)))
		{
			Definition = InDefinition;
		}
		else if (!ensureAlways(IsValid(Definition)))
		{
			return;
		}
		
		bInitialized = true;
		
		ForEachFragment([this](const UFragmentationFragment* Fragment)
		{
			Fragment->OnInstanceInitialized(this);
		});
	}
	
	template<typename T>
	T* Duplicate(UObject* Outer)
	{
		static_assert(std::is_base_of_v<UFragmentationInstance, T>, "T must inherit from UFragmentationInstance!");

		T* NewItemInstance = NewObject<T>(Outer);
		NewItemInstance->Initialize(Definition);

		if (!ensureAlways(IsValid(NewItemInstance)))
		{
			return nullptr;
		}

		for (FFragmentationLocalDataItem Data : LocalData.GetAllData())
		{
			NewItemInstance->GetLocalData().SetData(Data);
		}

		return NewItemInstance;
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

protected:
	void ForEachFragment(const TFunctionRef<void(const UFragmentationFragment*)>& Action) const;
	
private:
	UPROPERTY(EditAnywhere, Replicated)
	TSubclassOf<UFragmentationDefinition> Definition;

	UPROPERTY(Replicated)
	FFragmentationLocalData LocalData;

	bool bInitialized = false;
};
