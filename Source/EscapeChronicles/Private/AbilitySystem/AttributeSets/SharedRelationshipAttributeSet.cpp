// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/SharedRelationshipAttributeSet.h"

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
}

void USharedRelationshipAttributeSet::OnRep_MaxSuspicion(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxSuspicion, OldValue);
}