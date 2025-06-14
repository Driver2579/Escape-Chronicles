// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "CastActorsArrayToCharactersArrayStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FCastActorsArrayToCharactersArrayStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> Actors;

	UPROPERTY(VisibleAnywhere, Transient, Category="Output")
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> Characters;
};

/**
 * Casts a given array of actors to an array of characters.\n
 * @warning You must be sure that all actors in the array are the characters or the game will crash!
 */
USTRUCT(meta=(DisplayName="Cast Actors Array To Characters Array"))
struct FCastActorsArrayToCharactersArrayStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FCastActorsArrayToCharactersArrayStateTreeTask();

	using FInstanceDataType = FCastActorsArrayToCharactersArrayStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};