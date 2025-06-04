// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UVitalAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Health);
	DOREPLIFETIME(ThisClass, MaxHealth);

	DOREPLIFETIME(ThisClass, Energy);
	DOREPLIFETIME(ThisClass, MaxEnergy);

	DOREPLIFETIME(ThisClass, Cleanliness);
	DOREPLIFETIME(ThisClass, MaxCleanliness);
}

void UVitalAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::ClampAttribute(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		// Do not allow the Health to drop below 0 and above MaxHealth
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow MaxHealth to drop below 1
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetEnergyAttribute())
	{
		// Do not allow Energy to drop below 0 and above MaxEnergy
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxEnergy());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow MaxEnergy to drop below 1
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if (Attribute == GetCleanlinessAttribute())
	{
		// Do not allow Cleanliness to drop below 0 and above MaxCleanliness
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxCleanliness());
	}
	else if (Attribute == GetMaxCleanlinessAttribute())
	{
		// Do not allow MaxCleanliness to drop below 1
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

void UVitalAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue,
	const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent();

	if (Attribute == GetMaxHealthAttribute())
	{
		// Make sure current health is not greater than the new max health
		if (GetHealth() > NewValue)
		{
			OwningAbilitySystemComponent->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
	else if (Attribute == GetMaxEnergyAttribute())
	{
		// Make sure current energy is not greater than the new max energy
		if (GetEnergy() > NewValue)
		{
			OwningAbilitySystemComponent->ApplyModToAttribute(GetEnergyAttribute(), EGameplayModOp::Override, NewValue);
		}
	}
	else if (Attribute == GetMaxCleanlinessAttribute())
	{
		// Make sure current cleanliness is not greater than the new max cleanliness
		if (GetCleanliness() > NewValue)
		{
			OwningAbilitySystemComponent->ApplyModToAttribute(GetCleanlinessAttribute(), EGameplayModOp::Override,
				NewValue);
		}
	}
}

void UVitalAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldValue);
}

void UVitalAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldValue);
}

void UVitalAttributeSet::OnRep_Energy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Energy, OldValue);
}

void UVitalAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxEnergy, OldValue);
}

void UVitalAttributeSet::OnRep_Cleanliness(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Cleanliness, OldValue);
}

void UVitalAttributeSet::OnRep_MaxCleanliness(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxCleanliness, OldValue);
}