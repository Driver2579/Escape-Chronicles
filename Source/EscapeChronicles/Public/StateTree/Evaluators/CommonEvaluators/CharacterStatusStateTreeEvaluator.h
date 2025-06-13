// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "CharacterStatusStateTreeEvaluator.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FOnCharacterStatusChangedStateTreeEventPayload
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Transient)
	TObjectPtr<AEscapeChroniclesCharacter> Character;

	UPROPERTY(VisibleAnywhere, Transient)
	bool bNewStatus = false;

	bool IsValid() const
	{
		return Character.operator bool();
	}

	// Invalidates the payload and resets its properties to default values
	void Reset()
	{
		Character = nullptr;
		bNewStatus = false;
	}

	bool operator==(const FOnCharacterStatusChangedStateTreeEventPayload& Other) const
	{
		return bNewStatus == Other.bNewStatus && Character == Other.Character;
	}
};

USTRUCT()
struct FCharacterStatusStateTreeEvaluatorInstanceData
{
	GENERATED_BODY()

	/**
	 * The tag of the OnCharacterFaintedStatusChanged event. This event will be sent to the StateTree when any character
	 * has fainted or revived.
	 */
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTag OnCharacterFaintedStatusChangedEventTag;

	FDelegateHandle OnCharacterFaintedStatusChangedDelegateHandle;
};

/**
 * An evaluator that listens for character status events and sends a StateTree event when a character-status-related
 * event occurs.
 */
USTRUCT(DisplayName="Character Status")
struct FCharacterStatusStateTreeEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCharacterStatusStateTreeEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;

private:
	void OnCharacterFaintedStatusChanged(AEscapeChroniclesCharacter* Character, const bool bNewStatus) const;

	/**
	 * The last payload that was constructed from the last received OnCharacterFaintedStatusChanged event. It will be
	 * sent via the StateTree event in the next tick after the construction.
	 */
	mutable FOnCharacterStatusChangedStateTreeEventPayload LastCharacterFaintedStatusChangedPayload;

	/**
	 * A list of this evaluator's instances that already sent the LastCharacterFaintedStatusChangedPayload to the
	 * StateTree. This is used to avoid sending the same payload multiple times on the same instances. 🤡
	 */
	mutable TSet<const FInstanceDataType*> InstancesThatSentLastCharacterFaintedStatusChangedPayload;

	void ClearEvaluatorData(FInstanceDataType& InstanceData) const;
};