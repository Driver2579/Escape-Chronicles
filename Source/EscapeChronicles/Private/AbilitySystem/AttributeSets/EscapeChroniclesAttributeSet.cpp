// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/EscapeChroniclesAttributeSet.h"

void UEscapeChroniclesAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}