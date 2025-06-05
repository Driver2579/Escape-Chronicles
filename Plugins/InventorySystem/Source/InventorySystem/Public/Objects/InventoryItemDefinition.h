// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemFragments/InventoryItemFragment.h"
#include "InventoryItemDefinition.generated.h"

class UInventoryItemFragment;

// Describes inventory item instances by creating UInventoryItemFragment for them
UCLASS(Blueprintable, Const, Abstract)
class INVENTORYSYSTEM_API UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	const FText& GetName() { return DisplayName; }

	const TArray<UInventoryItemFragment*>& GetFragments() const { return Fragments; }


#if WITH_EDITOR
	virtual void PostCDOCompiled(const FPostCDOCompiledContext& Context) override
	{
		for (UInventoryItemFragment* Fragment : Fragments)
		{
			ensureAlwaysMsgf(Fragment->HasValidProperties(this), TEXT("The Fragment has invalid properties"));
		}
	}
#endif
	
private:
	UPROPERTY(EditDefaultsOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UInventoryItemFragment>> Fragments;
};
