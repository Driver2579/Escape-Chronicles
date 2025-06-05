// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "ActiveGameplayEffectHandle.h"
#include "CombatStateTreeEvaluator.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FOnPunchHitStateTreeEventPayload
{
	GENERATED_BODY()

	// The Ability System Component of the actor who performed the punch
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UAbilitySystemComponent> Instigator;

	// The Ability System Component of the actor who was hit by the punch
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UAbilitySystemComponent> Target;

	// Handle of the gameplay effect applied to the target as a result of the punch
	UPROPERTY(EditAnywhere, Category="Input")
	FActiveGameplayEffectHandle TargetAppliedEffectHandle;

	bool IsValid() const
	{
		return Instigator && Target;
	}

	// Invalidates the payload
	void Reset()
	{
		Instigator = nullptr;
		Target = nullptr;
		TargetAppliedEffectHandle.Invalidate();
	}

	bool operator==(const FOnPunchHitStateTreeEventPayload& Other) const
	{
		return Instigator == Other.Instigator && Target == Other.Target &&
			TargetAppliedEffectHandle == Other.TargetAppliedEffectHandle;
	}
};

USTRUCT()
struct FCombatStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	// The tag of the OnPunchHit event. This event will be sent to the StateTree when a punch hit occurs by any actor.
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTag OnPunchHitEventTag;

	FDelegateHandle OnPunchHitDelegateHandle;
};

// An evaluator that listens for combat events and sends a StateTree event when a combat-related event occurs
USTRUCT(DisplayName="Combat", Category="AI")
struct FCombatStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCombatStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;

private:
	void OnPunchHit(UAbilitySystemComponent* Instigator, UAbilitySystemComponent* Target,
		FActiveGameplayEffectHandle TargetAppliedEffectHandle) const;

	/**
	 * The last payload that was constructed from the last received OnPunchHit event. It will be sent via the StateTree
	 * event in the next tick after the construction.
	 */
	mutable FOnPunchHitStateTreeEventPayload LastPunchHitPayload;

	/**
	 * A list of this evaluator's instances that already sent the LastPunchHitPayload to the StateTree. This is used to
	 * avoid sending the same payload multiple times on the same instances. 🤡
	 */
	mutable TSet<const FInstanceDataType*> InstancesThatSentLastPunchHitPayload;

	void ClearEvaluatorData(FInstanceDataType& InstanceData) const;
};