// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FindSmartObjectByClassStateTreeTask.h"
#include "FindUnoccupiedActivitySpotSmartObjectByClassStateTreeTask.generated.h"

/**
 * Finds all activity spots of a specified class that are currently unoccupied and returns the first or nearest
 * unclaimed smart object in found activity spots.
 */
USTRUCT(DisplayName="Find Unoccupied Activity Spot Smart Object By Class", Category="Smart Objects")
struct FFindUnoccupiedActivitySpotSmartObjectByClassStateTreeTask : public FFindSmartObjectByClassStateTreeTask
{
	GENERATED_BODY()

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

protected:
	virtual void FilterFoundActors(FStateTreeExecutionContext& Context,
		TArray<AActor*>& InOutFoundActors) const override;
};