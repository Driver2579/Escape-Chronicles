// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objects/InventoryItemFragment.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "ClothingInventoryItemFragment.generated.h"

class AEscapeChroniclesCharacter;
class UGameplayEffect;

struct FStreamableHandle;
struct FGameplayTag;

/**
 * A fragment that replaces a mesh of the character with a specified one (it should be used for changing the clothes
 * (yeah, we really should have a real clothing system instead of replacing the mesh)) and applies a gameplay effect to
 * this character. The mesh and the gameplay effect will be added to the character when the item is added to the
 * specified type of the slot and removed when the item is removed from that slot. If the item is added to another type
 * than the specified one, then this fragment will do nothing.
 * @remark It is expected that an item with this fragment can't be added to a type of slots that has more than one slot.
 */
UCLASS()
class ESCAPECHRONICLES_API UClothingInventoryItemFragment : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual void OnItemAddedToSlot(UInventoryItemInstance* ItemInstance,
		UInventoryManagerComponent* InventoryManagerComponent, const FGameplayTag& SlotTypeTag,
		const int32 SlotIndex) const override;

	virtual void OnItemRemovedFromSlot(UInventoryItemInstance* ItemInstance,
		UInventoryManagerComponent* InventoryManagerComponent, const FGameplayTag& SlotTypeTag,
		const int32 SlotIndex) const override;

private:
	/**
	 * A tag that defines the type of the slot where this item will execute its logic. It would do nothing if the item
	 * was added to another type of the slot.
	 */
	UPROPERTY(EditAnywhere)
	FGameplayTag RequiredSlotTypeTag;

	// A mesh that will be set to the character when the item is equipped
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<USkeletalMesh> ClothingMesh;

	// Gameplay effect that will be applied to the character while the item is equipped
	UPROPERTY(EditAnywhere, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;

	mutable TMap<TWeakObjectPtr<UInventoryItemInstance>, TSharedPtr<FStreamableHandle>> LoadClothingMeshHandles;
	mutable TMap<TWeakObjectPtr<UInventoryItemInstance>, TSharedPtr<FStreamableHandle>> LoadGameplayEffectClassHandles;

	// Sets the loaded clothing mesh to the character
	void OnClothingMeshLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
		AEscapeChroniclesCharacter* Character) const;

	// Applies the loaded gameplay effect to the character
	void OnGameplayEffectClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
		AEscapeChroniclesCharacter* Character) const;

	mutable TMap<TWeakObjectPtr<AEscapeChroniclesCharacter>, FActiveGameplayEffectHandle> ActiveGameplayEffectHandles;
};