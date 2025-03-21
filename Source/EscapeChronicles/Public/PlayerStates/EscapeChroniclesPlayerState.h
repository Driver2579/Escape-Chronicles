// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"
#include "EscapeChroniclesPlayerState.generated.h"

class UAbilitySystemSet;

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

protected:
	virtual void PostInitializeComponents() override;

	UFUNCTION()
	virtual void OnPawnChanged(APlayerState* ThisPlayerState, APawn* NewPawn, APawn* OldPawn);

	/**
	 * Called when the new pawn is set (except of the situation when you switch between spectator pawns), but before the
	 * gameplay effects from AbilitySystemSets are applied. This should be overriden in child classes to initialize
	 * attributes with custom data if any.
	 */
	virtual void InitializeAttributes();

private:
	UPROPERTY(VisibleAnywhere, Category="Ability System")
	TObjectPtr<UEscapeChroniclesAbilitySystemComponent> AbilitySystemComponent;

	// Ability system sets to grant to this pawn's ability system
	UPROPERTY(EditDefaultsOnly, Category = "Ability System|Abilities")
	TArray<TObjectPtr<UAbilitySystemSet>> AbilitySystemSets;

	TWeakObjectPtr<APawn> LastNotSpectatorPawn;

	/**
	 * Initializes the MovementAttributeSet if it exists on the AbilitySystemComponent with CharacterMoverComponent's
	 * default values.
	 */
	void TryInitializeMovementAttributeSet();

	void OnMaxGroundSpeedChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec,
		const float EffectMagnitude, const float OldValue, const float NewValue) const;
};