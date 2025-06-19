// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryManagerFragment.h"
#include "RemoveItemsOnFaintedByClassificationInventoryManagerFragment.generated.h"

class UAbilitySystemComponent;
class AEscapeChroniclesCharacter;

enum class EItemClassification : uint8;

// A fragment that removes items of the given classifications from the inventory when its owning character faints
UCLASS(DisplayName="Remove Items On Fainted By Classification")
class ESCAPECHRONICLES_API URemoveItemsOnFaintedByClassificationInventoryManagerFragment :
	public UInventoryManagerFragment
{
	GENERATED_BODY()

protected:
	virtual void OnManagerInitialized() override;

	virtual void OnOwningPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState);

private:
	// Items with these classifications will be removed from the inventory when the character faints
	UPROPERTY(EditAnywhere)
	TSet<EItemClassification> ClassificationsToRemove;

	// If the character has one of these gameplay tags, he will not have items removed from the inventory
	UPROPERTY(EditAnywhere, Category="Gameplay Tags")
	FGameplayTagContainer ImmunityTags;

	FDelegateHandle OnFaintedGameplayTagChangedDelegateHandle;

	/**
	 * Removes all items in the inventory that have a classification matching one of the specified in
	 * ClassificationsToRemove if the character has fainted unless he has one of the immunity gameplay tags.
	 */
	void OnFaintedGameplayTagChanged(FGameplayTag GameplayTag, int32 NewCount,
		UAbilitySystemComponent* AbilitySystemComponent) const;
};