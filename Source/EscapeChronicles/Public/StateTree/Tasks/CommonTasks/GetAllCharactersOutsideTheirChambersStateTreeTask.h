// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetAllCharactersOutsideTheirChambersStateTreeTask.generated.h"

class AEscapeChroniclesCharacter;

USTRUCT()
struct FGetAllCharactersOutsideTheirChambersStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Found characters that own any prisoner chambers and aren't currently in any of them
	UPROPERTY(EditAnywhere, Category="Output")
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> OutCharactersOutsideTheirChambers;

	// If the character has at least one of these tags, then he will not be considered as the one outside his chamber
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTagContainer AllowedGameplayTags;
};

/**
 * Finds all characters that are currently in the game and that own any prisoner chambers but aren't currently in any of
 * them. Returns Failed if no such characters were found.
 */
USTRUCT(meta=(DisplayName="Get All Characters Outside Their Chambers"))
struct FGetAllCharactersOutsideTheirChambersStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetAllCharactersOutsideTheirChambersStateTreeTask();

	using FInstanceDataType = FGetAllCharactersOutsideTheirChambersStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};