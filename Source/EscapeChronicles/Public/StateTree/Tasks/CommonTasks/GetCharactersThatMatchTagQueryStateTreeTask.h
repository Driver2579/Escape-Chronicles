// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetCharactersThatMatchTagQueryStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FGetCharactersThatMatchTagQueryStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Actors to search for the characters in
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<AActor>> ActorsToSearchIn;

	// Found characters that match the tag query
	UPROPERTY(EditAnywhere, Category="Output")
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> OutSuspiciousCharacters;

	// The tag query to match characters against
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTagQuery GameplayTagQuery;
};

/**
 * Finds characters in the given array of actors that match the given tag query. Returns Failed if no characters that
 * match the tag query were found.
 */
USTRUCT(meta=(DisplayName="Get Characters That Match Tag Query"))
struct FGetCharactersThatMatchTagQueryStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetCharactersThatMatchTagQueryStateTreeTask();

	using FInstanceDataType = FGetCharactersThatMatchTagQueryStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};