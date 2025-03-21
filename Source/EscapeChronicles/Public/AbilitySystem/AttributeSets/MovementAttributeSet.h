// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesAttributeSet.h"
#include "MovementAttributeSet.generated.h"

// AttributeSet for movement-related attributes
UCLASS()
class ESCAPECHRONICLES_API UMovementAttributeSet : public UEscapeChroniclesAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(ThisClass, MaxGroundSpeed);

	mutable FAttributeChangedDelegate OnMaxGroundSpeedChanged;

protected:
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const override;

private:
	UPROPERTY(Transient, ReplicatedUsing="OnRep_MaxGroundSpeed")
	FGameplayAttributeData MaxGroundSpeed;

	UFUNCTION()
	void OnRep_MaxGroundSpeed(const FGameplayAttributeData& OldValue) const;

	float MaxGroundSpeedBeforeChangeByGameplayEffect = 0;
};