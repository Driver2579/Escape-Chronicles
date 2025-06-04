// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesAttributeSet.h"
#include "SharedRelationshipAttributeSet.generated.h"

// AttributeSet for attributes related to relationships with NPCs and shared between all of NPCs (e.g., suspicion ඞ)
UCLASS()
class ESCAPECHRONICLES_API USharedRelationshipAttributeSet : public UEscapeChroniclesAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_GETTERS(ThisClass, Suspicion);
	ATTRIBUTE_GETTERS(ThisClass, MaxSuspicion);

protected:
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

private:
	UPROPERTY(Transient, ReplicatedUsing="OnRep_Suspicion")
	FGameplayAttributeData Suspicion = 0;

	UPROPERTY(Transient, ReplicatedUsing="OnRep_MaxSuspicion")
	FGameplayAttributeData MaxSuspicion = 0;

	UFUNCTION()
	void OnRep_Suspicion(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxSuspicion(const FGameplayAttributeData& OldValue) const;
};