// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AttributeSets/EscapeChroniclesAttributeSet.h"

#include "GameplayEffectExtension.h"

bool UEscapeChroniclesAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	const UEscapeChroniclesAbilitySystemComponent* AbilitySystemComponent =
		GetOwningEscapeChroniclesAbilitySystemComponent();

#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Check if we can modify the attribute. If not, the GameplayEffect's modification will be ignored.
	return AbilitySystemComponent->CanModifyAttribute(Data.EvaluatedData.Attribute);
}

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