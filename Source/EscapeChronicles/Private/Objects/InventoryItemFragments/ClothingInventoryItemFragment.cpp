// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/ClothingInventoryItemFragment.h"

#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"
#include "Objects/InventoryItemInstance.h"

void UClothingInventoryItemFragment::OnItemAddedToSlot(UInventoryItemInstance* ItemInstance,
	UInventoryManagerComponent* InventoryManagerComponent, const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
{
	Super::OnItemAddedToSlot(ItemInstance, InventoryManagerComponent, SlotTypeTag, SlotIndex);

	// If the item was added to another slot than the required one, then don't do anything
	if (SlotTypeTag != RequiredSlotTypeTag)
	{
		return;
	}

#if DO_ENSURE
	ensureAlwaysMsgf(SlotIndex == 0,
		TEXT("It is expected that an item with this fragment can't be added to a type of slots that has more "
			"than one slot!"));
#endif

#if DO_CHECK
	check(IsValid(InventoryManagerComponent));
	check(IsValid(ItemInstance));
#endif

	AEscapeChroniclesCharacter* Character = InventoryManagerComponent->GetOwner<AEscapeChroniclesCharacter>();

	if (!IsValid(Character))
	{
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

	// Load the clothing mesh and remember its handle to release it later
	if (ensureAlways(!ClothingMesh.IsNull()))
	{
		const TSharedPtr<FStreamableHandle> LoadClothingMeshHandle = StreamableManager.RequestAsyncLoad(
			ClothingMesh.ToSoftObjectPath(),
			FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnClothingMeshLoaded,
				Character));

		LoadClothingMeshHandles.Add(ItemInstance, LoadClothingMeshHandle);
	}

	// Do the same thing with the gameplay effect class
	if (ensureAlways(!GameplayEffectClass.IsNull()))
	{
		const TSharedPtr<FStreamableHandle> LoadGameplayEffectClassHandle = StreamableManager.RequestAsyncLoad(
			GameplayEffectClass.ToSoftObjectPath(),
			FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnGameplayEffectClassLoaded,
				Character));

		LoadGameplayEffectClassHandles.Add(ItemInstance, LoadGameplayEffectClassHandle);
	}
}

// ReSharper disable once CppPassValueParameterByConstReference
void UClothingInventoryItemFragment::OnClothingMeshLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
	AEscapeChroniclesCharacter* Character) const
{
	// Set the loaded clothing mesh to the character
	if (ensureAlways(IsValid(Character)))
	{
		Character->SetMesh(ClothingMesh.Get());
	}

#if DO_CHECK
	check(LoadObjectHandle.IsValid())
#endif

	// Release the loaded mesh because we don't need to keep it here anymore
	LoadObjectHandle->ReleaseHandle();
}

// ReSharper disable once CppPassValueParameterByConstReference
void UClothingInventoryItemFragment::OnGameplayEffectClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
	AEscapeChroniclesCharacter* Character) const
{
	// Apply the loaded gameplay effect to the character
	if (ensureAlways(IsValid(Character)))
	{
		UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

		if (ensureAlways(IsValid(AbilitySystemComponent)))
		{
			const FActiveGameplayEffectHandle ActiveGameplayEffectHandle =
				AbilitySystemComponent->ApplyGameplayEffectToSelf(
					GameplayEffectClass->GetDefaultObject<UGameplayEffect>(), UGameplayEffect::INVALID_LEVEL,
					AbilitySystemComponent->MakeEffectContext());

			// Remember the handle of the active gameplay effect to remove it later
			ActiveGameplayEffectHandles.Add(Character, ActiveGameplayEffectHandle);
		}
	}

#if DO_CHECK
	check(LoadObjectHandle.IsValid())
#endif

	// Release the loaded gameplay effect class because we don't need to keep it here anymore
	LoadObjectHandle->ReleaseHandle();
}

void UClothingInventoryItemFragment::OnItemRemovedFromSlot(UInventoryItemInstance* ItemInstance,
	UInventoryManagerComponent* InventoryManagerComponent, const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
{
	// If the item was removed from another slot than the required one, then don't do anything
	if (SlotTypeTag != RequiredSlotTypeTag)
	{
		return;
	}

#if DO_ENSURE
	ensureAlwaysMsgf(SlotIndex == 0,
		TEXT("It is expected that an item with this fragment can't be added to a type of slots that has more "
			"than one slot!"));
#endif

#if DO_CHECK
	check(IsValid(ItemInstance));
#endif

	AEscapeChroniclesCharacter* Character = InventoryManagerComponent->GetOwner<AEscapeChroniclesCharacter>();

	/**
	 * If the item was removed from the character and the character is still valid, then reset his mesh back and remove
	 * the gameplay effect we applied to him.
	 */
	if (IsValid(Character))
	{
		Character->ResetMesh();

		UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

		if (IsValid(AbilitySystemComponent))
		{
			const FActiveGameplayEffectHandle* ActiveGameplayEffectHandle = ActiveGameplayEffectHandles.Find(
				Character);

			if (ActiveGameplayEffectHandle)
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(*ActiveGameplayEffectHandle);
			}
		}
	}

	// Forget about the handle of the loaded clothing mesh and cancel it if it is still loading
	TSharedPtr<FStreamableHandle> LoadClothingMeshHandle;
	LoadClothingMeshHandles.RemoveAndCopyValue(ItemInstance, LoadClothingMeshHandle);

	// Cancel loading the clothing mesh if it's still loading
	if (LoadClothingMeshHandle.IsValid())
	{
		LoadClothingMeshHandle->CancelHandle();
	}

	// Forget about the handle of the loaded gameplay effect class and cancel it if it is still loading
	TSharedPtr<FStreamableHandle> LoadGameplayEffectClassHandle;
	LoadGameplayEffectClassHandles.RemoveAndCopyValue(ItemInstance, LoadGameplayEffectClassHandle);

	// Cancel loading the gameplay effect if it's still loading
	if (LoadGameplayEffectClassHandle.IsValid())
	{
		LoadGameplayEffectClassHandle->CancelHandle();
	}

	// Forget about the handle of the applied gameplay effect for the character
	ActiveGameplayEffectHandles.Remove(Character);

	Super::OnItemRemovedFromSlot(ItemInstance, InventoryManagerComponent, SlotTypeTag, SlotIndex);
}