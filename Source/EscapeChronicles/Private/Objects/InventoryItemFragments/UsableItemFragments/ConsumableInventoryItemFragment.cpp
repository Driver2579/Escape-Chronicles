// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/UsableItemFragments/ConsumableInventoryItemFragment.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Engine/AssetManager.h"

bool UConsumableInventoryItemFragment::OnTriedToUseItem(UInventoryManagerComponent* InventoryManagerComponent,
	UInventoryItemInstance* ItemInstance, const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
{
#if DO_CHECK
	check(IsValid(InventoryManagerComponent));
	check(IsValid(ItemInstance));
#endif

	// Try to get the ASC from the owning actor
	UAbilitySystemComponent* OwningAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(
		InventoryManagerComponent->GetOwner());

	// We can't use this item if either the ASC is invalid or the gameplay effect class isn't set
	if (!IsValid(OwningAbilitySystemComponent) || !ensureAlways(!GameplayEffectClass.IsNull()))
	{
		return false;
	}

	// Load and apply the gameplay effect class to the ASC
	UAssetManager::GetStreamableManager().RequestAsyncLoad(GameplayEffectClass.ToSoftObjectPath(),
		FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnGameplayEffectClassLoaded,
			OwningAbilitySystemComponent));

	return true;
}

// ReSharper disable once CppPassValueParameterByConstReference
void UConsumableInventoryItemFragment::OnGameplayEffectClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
	UAbilitySystemComponent* OwningAbilitySystemComponent) const
{
	// Ensure the given ASC didn't become invalid while we were loading the gameplay effect class
	if (!ensureAlways(IsValid(OwningAbilitySystemComponent)))
	{
		return;
	}

#if DO_CHECK
	check(LoadObjectHandle.IsValid());
	check(GameplayEffectClass.IsValid());
#endif

	// Apply the gameplay effect with the specified level to the ASC
	OwningAbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffectClass->GetDefaultObject<UGameplayEffect>(),
		GameplayEffectLevel, OwningAbilitySystemComponent->MakeEffectContext());

	// Release the loaded gameplay effect class because we don't need to keep it here anymore
	LoadObjectHandle->ReleaseHandle();
}