// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreePropertyFunctionBase.h"
#include "TryGetActivitySpotStateTreePropertyFunction.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FTryGetActivitySpotStateTreePropertyFunctionInstanceData
{
	GENERATED_BODY()

	// A character to get the activity spot from
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<AEscapeChroniclesCharacter> Character;

	/**
	 * Either the activity spot that the character is currently at, or the character itself if it isn't currently at the
	 * activity spot.
	 */
	UPROPERTY(VisibleAnywhere, Transient, Category="Output")
	TObjectPtr<AActor> OutCharacterOrActivitySpot;
};

/**
 * A property function that receives a character and tries to get the activity spot this character is currently at. If
 * the character is not at an activity spot, it will return the given character itself if it isn't at the activity spot.
 */
USTRUCT(DisplayName="Try Get Activity Spot")
struct FTryGetActivitySpotStateTreePropertyFunction : public FStateTreePropertyFunctionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTryGetActivitySpotStateTreePropertyFunctionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Execute(FStateTreeExecutionContext& Context) const override;
};