// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FindSmartObjectByClassStateTreeTask.h"
#include "FindOwningSmartObjectByClassStateTreeTask.generated.h"

/**
 * Finds all actors of a specified class and returns the first or nearest unclaimed smart object in found actors that
 * the UserActor owns (an actor that owns the smart object should have a PlayerOwnershipComponent), and the UserActor
 * itself must be an EscapeChroniclesCharacter.
 */
USTRUCT(Category="Smart Objects", meta=(DisplayName="Find Owning Smart Object by Class"))
struct FFindOwningSmartObjectByClassStateTreeTask : public FFindSmartObjectByClassStateTreeTask
{
	GENERATED_BODY()

	// Overriden to filter found actors based on ownership (only actors that the UserActor owns will be left)
	virtual void FilterFoundActors(FStateTreeExecutionContext& Context,
		TArray<AActor*>& InOutFoundActors) const override;
};