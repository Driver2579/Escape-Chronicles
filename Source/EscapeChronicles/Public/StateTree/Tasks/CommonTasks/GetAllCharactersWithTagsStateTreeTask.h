// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GetAllCharactersWithTagsStateTreeTask.generated.h"

USTRUCT()
struct FGetAllCharactersWithTagsStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// A tag that character's ASC must have to be included in the output array
	UPROPERTY(EditAnywhere, Category="Parameter")
	FGameplayTag Tag;

	/**
	 * Characters that have the specified tags in their ASCs. This array is a type of AActor to be able to pass it as a
	 * parameter to other nodes or state trees.
	 */
	UPROPERTY(VisibleAnywhere, Category="Output")
	TArray<TObjectPtr<AActor>> OutCharacters;
};

/**
 * Iterates all PlayerStates in the world, leaving only those that have ASCs with the specified tags, and returns their
 * character actors. If no characters are found, the task fails.
 */
USTRUCT(meta=(DisplayName="Get All Characters With Tags"))
struct FGetAllCharactersWithTagsStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FGetAllCharactersWithTagsStateTreeTask();

	using FInstanceDataType = FGetAllCharactersWithTagsStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};