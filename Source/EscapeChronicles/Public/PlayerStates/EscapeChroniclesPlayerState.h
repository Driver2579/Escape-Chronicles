// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"
#include "EscapeChroniclesPlayerState.generated.h"

class UInventoryManagerComponent;
class UAbilitySystemSet;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEscapeChroniclesPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override final
	{
		return AbilitySystemComponent;
	}

	virtual UInventoryManagerComponent* GetInventoryManagerComponent() const
	{
		return InventoryManagerComponent;
	}

	UEscapeChroniclesAbilitySystemComponent* GetEscapeChroniclesAbilitySystemComponent() const
	{
		return AbilitySystemComponent;
	}

protected:
	virtual void PostInitializeComponents() override;

private:
	UPROPERTY(VisibleAnywhere, Category="Ability System")
	TObjectPtr<UEscapeChroniclesAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category="Inventory")
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;

	// Ability system sets to grant to this pawn's ability system
	UPROPERTY(EditDefaultsOnly, Category = "Ability System|Abilities")
	TArray<TObjectPtr<UAbilitySystemSet>> AbilitySystemSets;
};