// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetSuspiciousCharactersStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FGetSuspiciousCharactersStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Actors to search for the suspicious characters in
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> ActorsToSearchIn;

	// Found suspicious characters
	UPROPERTY(EditAnywhere, Category="Output")
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> OutSuspiciousCharacters;

	// The minimum suspicious value to consider a character suspicious
	UPROPERTY(EditAnywhere, Category="Parameter")
	float MinSuspicious = 90;
};

/**
 * Finds characters in the given array of actors that have the suspicion attribute value above a specified one. Returns
 * Failed if no suspicious characters were found.
 */
USTRUCT(meta=(DisplayName="Get Suspicious Characters"))
struct FGetSuspiciousCharactersStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetSuspiciousCharactersStateTreeTask();

	using FInstanceDataType = FGetSuspiciousCharactersStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};