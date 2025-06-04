// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/CombatAttributeSet.h"

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

void UCombatAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Damage, OldValue);
}