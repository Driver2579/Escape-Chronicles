// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"

#include "GameplayEffectExtension.h"
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

bool UVitalAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// === Remember the attributes' values before the gameplay effect is applied ===

	HealthBeforeChangeByGameplayEffect = GetHealth();
	MaxHealthBeforeChangeByGameplayEffect = GetMaxHealth();

	EnergyBeforeChangeByGameplayEffect = GetEnergy();
	MaxEnergyBeforeChangeByGameplayEffect = GetMaxEnergy();

	CleanlinessBeforeChangeByGameplayEffect = GetCleanliness();
	MaxCleanlinessBeforeChangeByGameplayEffect = GetMaxCleanliness();

	return true;
}

void UVitalAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();

	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	// === Broadcast an event of the attributes that have been changed ===

	if (HealthBeforeChangeByGameplayEffect != GetHealth())
	{
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			HealthBeforeChangeByGameplayEffect, GetHealth());

		// Check if we're out of health
		if (!bOutOfHealth && GetHealth() <= 0)
		{
			OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
				HealthBeforeChangeByGameplayEffect, GetHealth());

			bOutOfHealth = true;
		}
	}

	if (MaxHealthBeforeChangeByGameplayEffect != GetMaxHealth())
	{
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			MaxHealthBeforeChangeByGameplayEffect, GetMaxHealth());
	}

	if (EnergyBeforeChangeByGameplayEffect != GetEnergy())
	{
		OnEnergyChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			EnergyBeforeChangeByGameplayEffect, GetEnergy());

		// Check if we're out of energy
		if (!bOutOfEnergy && GetEnergy() <= 0)
		{
			OnOutOfEnergy.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
				EnergyBeforeChangeByGameplayEffect, GetEnergy());

			bOutOfEnergy = true;
		}
	}

	if (MaxEnergyBeforeChangeByGameplayEffect != GetMaxEnergy())
	{
		OnMaxEnergyChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			MaxEnergyBeforeChangeByGameplayEffect, GetMaxEnergy());
	}

	if (CleanlinessBeforeChangeByGameplayEffect != GetCleanliness())
	{
		OnCleanlinessChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			CleanlinessBeforeChangeByGameplayEffect, GetCleanliness());
	}

	if (MaxCleanlinessBeforeChangeByGameplayEffect != GetMaxCleanliness())
	{
		OnMaxCleanlinessChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			MaxCleanlinessBeforeChangeByGameplayEffect, GetMaxCleanliness());
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

void UVitalAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldValue);

	const float CurrentHealth = GetHealth();
	const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
	
	OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

	if (!bOutOfHealth && CurrentHealth <= 0)
	{
		OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(),
			CurrentHealth);
	}

	bOutOfHealth = CurrentHealth <= 0;
}

void UVitalAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxHealth, OldValue);

	OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxHealth() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(), GetMaxHealth());
}

void UVitalAttributeSet::OnRep_Energy(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Energy, OldValue);

	const float CurrentEnergy = GetEnergy();
	const float EstimatedMagnitude = CurrentEnergy - OldValue.GetCurrentValue();
	
	OnEnergyChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentEnergy);

	if (!bOutOfEnergy && CurrentEnergy <= 0)
	{
		OnOutOfEnergy.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(),
			CurrentEnergy);
	}

	bOutOfEnergy = CurrentEnergy <= 0;
}

void UVitalAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxEnergy, OldValue);

	OnMaxEnergyChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxEnergy() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(), GetMaxEnergy());
}

void UVitalAttributeSet::OnRep_Cleanliness(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Cleanliness, OldValue);
	
	OnMaxEnergyChanged.Broadcast(nullptr, nullptr, nullptr, GetCleanliness() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(), GetCleanliness());
}

void UVitalAttributeSet::OnRep_MaxCleanliness(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxCleanliness, OldValue);

	OnMaxCleanlinessChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxCleanliness() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(), GetMaxCleanliness());
}