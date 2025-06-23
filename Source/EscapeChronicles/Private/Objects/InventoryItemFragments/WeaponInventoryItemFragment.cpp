// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Objects/InventoryItemFragments/WeaponInventoryItemFragment.h"
#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"

void UWeaponInventoryItemFragment::StartHolding(UInventoryItemInstance* ItemInstance) const 
{
	Super::StartHolding(ItemInstance);

	if (ItemInstance->GetWorld()->GetNetMode() >= NM_Client) return;
	
	if (!ensureAlways(!WeaponGameplayEffectData.Contains(ItemInstance))) return;

	const TSharedPtr<FStreamableHandle> LoadedHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		HoldingGameplayEffectClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::WeaponGameplayEffectLoaded, ItemInstance));

	FWeaponGameplayEffectData Data;
	Data.StreamableHandle = LoadedHandle;

	WeaponGameplayEffectData.Add(ItemInstance, Data);
}

void UWeaponInventoryItemFragment::StopHolding(UInventoryItemInstance* ItemInstance) const
{
	Super::StopHolding(ItemInstance);

	if (ItemInstance->GetWorld()->GetNetMode() >= NM_Client) return;

	if (!ensureAlways(WeaponGameplayEffectData.Contains(ItemInstance))) return;

	const UInventoryManagerComponent* Inventory = Cast<UInventoryManagerComponent>(ItemInstance->GetOuter());
	if (!ensureAlways(IsValid(Inventory))) return;

	const AEscapeChroniclesCharacter* Character = Inventory->GetOwner<AEscapeChroniclesCharacter>();
	if (!ensureAlways(IsValid(Character))) return;

	UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();
	if (!IsValid(AbilitySystemComponent)) return;

	AbilitySystemComponent->RemoveActiveGameplayEffect(
		WeaponGameplayEffectData[ItemInstance].ActiveGameplayEffectHandle);

	WeaponGameplayEffectData[ItemInstance].StreamableHandle->CancelHandle();
	WeaponGameplayEffectData[ItemInstance].StreamableHandle.Reset();

	WeaponGameplayEffectData.Remove(ItemInstance);
}

void UWeaponInventoryItemFragment::WeaponGameplayEffectLoaded(UInventoryItemInstance* ItemInstance) const
{
	const UInventoryManagerComponent* Inventory = Cast<UInventoryManagerComponent>(ItemInstance->GetOuter());
	if (!ensureAlways(IsValid(Inventory))) return;

	const AEscapeChroniclesCharacter* Character = Inventory->GetOwner<AEscapeChroniclesCharacter>();
	if (!ensureAlways(IsValid(Character))) return;

	UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();
	if (!ensureAlways(IsValid(AbilitySystemComponent))) return;

	const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		HoldingGameplayEffectClass.Get(), 1, AbilitySystemComponent->MakeEffectContext());

	WeaponGameplayEffectData[ItemInstance].ActiveGameplayEffectHandle =
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get(),
			AbilitySystemComponent->GetPredictionKeyForNewAction());

#if DO_ENSURE
	ensureAlways(WeaponGameplayEffectData[ItemInstance].ActiveGameplayEffectHandle.IsValid());
#endif
}
