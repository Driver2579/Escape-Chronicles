// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/CombatAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Damage);
}

void UCombatAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::ClampAttribute(Attribute, NewValue);

	if (Attribute == GetDamageAttribute())
	{
		// Do not allow the Damage to drop below 1
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

bool UCombatAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// === Remember the attributes' values before the gameplay effect is applied ===

	DamageBeforeChangeByGameplayEffect = GetDamage();

	return true;
}

void UCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();

	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	// === Broadcast an event of the attributes that have been changed ===

	if (DamageBeforeChangeByGameplayEffect != GetDamage())
	{
		OnDamageChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			DamageBeforeChangeByGameplayEffect, GetDamage());
	}
}

void UCombatAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Damage, OldValue);

	OnDamageChanged.Broadcast(nullptr, nullptr, nullptr, GetDamage() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(), GetDamage());
}