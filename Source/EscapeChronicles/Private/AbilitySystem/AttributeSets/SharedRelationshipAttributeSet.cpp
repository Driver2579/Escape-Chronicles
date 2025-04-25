// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/SharedRelationshipAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void USharedRelationshipAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Suspicion);
	DOREPLIFETIME(ThisClass, MaxSuspicion);
}

void USharedRelationshipAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::ClampAttribute(Attribute, NewValue);

	if (Attribute == GetSuspicionAttribute())
	{
		// Do not allow the Suspicion to drop below 0 and above MaxSuspicion
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSuspicion());
	}
	else if (Attribute == GetMaxSuspicionAttribute())
	{
		// Do not allow MaxSuspicion to drop below 1
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

bool USharedRelationshipAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// === Remember the attributes' values before the gameplay effect is applied ===

	SuspicionBeforeChangeByGameplayEffect = GetSuspicion();
	MaxSuspicionBeforeChangeByGameplayEffect = GetMaxSuspicion();

	return true;
}

void USharedRelationshipAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();

	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	// === Broadcast an event of the attributes that have been changed ===

	if (SuspicionBeforeChangeByGameplayEffect != GetSuspicion())
	{
		OnSuspicionChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			SuspicionBeforeChangeByGameplayEffect, GetSuspicion());
	}

	if (MaxSuspicionBeforeChangeByGameplayEffect != GetMaxSuspicion())
	{
		OnMaxSuspicionChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			MaxSuspicionBeforeChangeByGameplayEffect, GetMaxSuspicion());
	}
}

void USharedRelationshipAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue,
	const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UAbilitySystemComponent* OwningAbilitySystemComponent = GetOwningAbilitySystemComponent();

	if (Attribute == GetMaxSuspicionAttribute())
	{
		// Make sure current suspicion is not greater than the new max suspicion
		if (GetSuspicion() > NewValue)
		{
			OwningAbilitySystemComponent->ApplyModToAttribute(GetSuspicionAttribute(), EGameplayModOp::Override,
				NewValue);
		}
	}
}

void USharedRelationshipAttributeSet::OnRep_Suspicion(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Suspicion, OldValue);

	OnSuspicionChanged.Broadcast(nullptr, nullptr, nullptr, GetSuspicion() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(), GetSuspicion());
}

void USharedRelationshipAttributeSet::OnRep_MaxSuspicion(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxSuspicion, OldValue);

	OnMaxSuspicionChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxSuspicion() - OldValue.GetCurrentValue(),
		OldValue.GetCurrentValue(), GetMaxSuspicion());
}