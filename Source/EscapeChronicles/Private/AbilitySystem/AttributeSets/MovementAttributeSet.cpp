// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/MovementAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void UMovementAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MaxGroundSpeed);
}

bool UMovementAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	// Remember the MaxGroundSpeed before the gameplay effect is applied
	MaxGroundSpeedBeforeChangeByGameplayEffect = GetMaxGroundSpeed();

	return Super::PreGameplayEffectExecute(Data);
}

void UMovementAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();

	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	// Broadcast the event if the MaxGroundSpeed has changed
	if (MaxGroundSpeedBeforeChangeByGameplayEffect != GetMaxGroundSpeed())
	{
		OnMaxGroundSpeedChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
			MaxGroundSpeedBeforeChangeByGameplayEffect, GetMaxGroundSpeed());
	}
}

void UMovementAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue,
	const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxGroundSpeedAttribute())
	{
		OnMaxGroundSpeedChanged.Broadcast(nullptr, nullptr, nullptr, NewValue - OldValue, OldValue, NewValue);
	}
}

void UMovementAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::ClampAttribute(Attribute, NewValue);

	if (Attribute == GetMaxGroundSpeedAttribute())
	{
		// Do not allow the MaxGroundSpeed to drop below 1
		constexpr float MinimumSpeed = 1;
		NewValue = FMath::Max(MinimumSpeed, NewValue);
	}
}

void UMovementAttributeSet::OnRep_MaxGroundSpeed(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxGroundSpeed, OldValue);
}