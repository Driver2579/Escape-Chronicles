// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/EscapeChroniclesAttributeSet.h"

void UEscapeChroniclesAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UEscapeChroniclesAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}