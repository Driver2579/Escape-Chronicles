﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesAttributeSet.h"
#include "CombatAttributeSet.generated.h"

// AttributeSet for attributes related to combat (e.g., damage)
UCLASS()
class ESCAPECHRONICLES_API UCombatAttributeSet : public UEscapeChroniclesAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_GETTERS(ThisClass, Damage);

protected:
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;

private:
	UPROPERTY(Transient, ReplicatedUsing="OnRep_Damage")
	FGameplayAttributeData Damage;

	UFUNCTION()
	void OnRep_Damage(const FGameplayAttributeData& OldValue) const;
};