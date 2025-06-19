// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "EnvironmentInteractionStateTreeEvaluator.generated.h"

class UDestructibleComponent;
class AEscapeChroniclesCharacter;

USTRUCT()
struct FOnDestructibleDamagedStateTreeEventPayload
{
	GENERATED_BODY()

	// A character that damaged the destructible component
	UPROPERTY(VisibleAnywhere, Transient)
	TObjectPtr<AEscapeChroniclesCharacter> Character;

	// A destructible component that was damaged
	UPROPERTY(VisibleAnywhere, Transient)
	TObjectPtr<UDestructibleComponent> DestructibleComponent;

	bool IsValid() const
	{
		return Character && DestructibleComponent;
	}

	// Invalidates the payload
	void Reset()
	{
		Character = nullptr;
		DestructibleComponent = nullptr;
	}

	bool operator==(const FOnDestructibleDamagedStateTreeEventPayload& Other) const
	{
		return Character == Other.Character && DestructibleComponent == Other.DestructibleComponent;
	}
};

USTRUCT()
struct FEnvironmentInteractionStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	/**
	 * The tag of the OnDestructibleDamaged event. This event will be sent to the StateTree when the destructible
	 * component is damaged by any character.
	 */
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTag OnDestructibleDamagedEventTag;

	FDelegateHandle OnDestructibleDamagedDelegateHandle;
};

/**
 * An evaluator that listens for environment interaction events and sends a StateTree event when an
 * environment-interaction-related event occurs.
 */
USTRUCT(DisplayName="Environment Interaction", Category="AI")
struct FEnvironmentInteractionStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FEnvironmentInteractionStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;

private:
	void OnDestructibleDamaged(AEscapeChroniclesCharacter* Character,
		UDestructibleComponent* DestructibleComponent) const;

	/**
	 * The last payload that was constructed from the last received OnDestructibleHit event. It will be sent via the
	 * StateTree event in the next tick after the construction.
	 */
	mutable FOnDestructibleDamagedStateTreeEventPayload LastDestructibleHitPayload;

	/**
	 * A list of this evaluator's instances that already sent the LastDestructibleHitPayload to the StateTree. This is
	 * used to avoid sending the same payload multiple times on the same instances. 🤡
	 */
	mutable TSet<const FInstanceDataType*> InstancesThatSentLastDestructibleDamagedPayload;

	void ClearEvaluatorData(FInstanceDataType& InstanceData) const;
};