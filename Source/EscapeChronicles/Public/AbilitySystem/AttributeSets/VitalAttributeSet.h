﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesAttributeSet.h"
#include "VitalAttributeSet.generated.h"

// AttributeSet for vital-related attributes (e.g., health etc.)
UCLASS()
class ESCAPECHRONICLES_API UVitalAttributeSet : public UEscapeChroniclesAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_GETTERS(ThisClass, Health);
	ATTRIBUTE_GETTERS(ThisClass, MaxHealth);

	ATTRIBUTE_GETTERS(ThisClass, Energy);
	ATTRIBUTE_GETTERS(ThisClass, MaxEnergy);

	ATTRIBUTE_GETTERS(ThisClass, Cleanliness);
	ATTRIBUTE_GETTERS(ThisClass, MaxCleanliness);

protected:
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

private:
	UPROPERTY(Transient, ReplicatedUsing="OnRep_Health")
	FGameplayAttributeData Health = 100;

	UPROPERTY(Transient, ReplicatedUsing="OnRep_MaxHealth")
	FGameplayAttributeData MaxHealth = 100;

	UPROPERTY(Transient, ReplicatedUsing="OnRep_Energy")
	FGameplayAttributeData Energy = 100;

	UPROPERTY(Transient, ReplicatedUsing="OnRep_MaxEnergy")
	FGameplayAttributeData MaxEnergy = 100;

	UPROPERTY(Transient, ReplicatedUsing="OnRep_Cleanliness")
	FGameplayAttributeData Cleanliness = 100;

	UPROPERTY(Transient, ReplicatedUsing="OnRep_MaxCleanliness")
	FGameplayAttributeData MaxCleanliness = 100;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Energy(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxEnergy(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_Cleanliness(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxCleanliness(const FGameplayAttributeData& OldValue) const;
};