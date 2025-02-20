// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/EscapeChroniclesAbilitySystemComponent.h"
#include "EscapeChroniclesPlayerState.generated.h"

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEscapeChroniclesPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override final
	{
		return AbilitySystemComponent;
	}

	UEscapeChroniclesAbilitySystemComponent* GetEscapeChroniclesAbilitySystemComponent() const
	{
		return AbilitySystemComponent;
	}

private:
	UPROPERTY(VisibleAnywhere, Category="Ability System")
	TObjectPtr<UEscapeChroniclesAbilitySystemComponent> AbilitySystemComponent;
};