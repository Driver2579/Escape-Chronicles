// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "GameplayTagContainer.h"
#include "EscapeChroniclesPlayerStart.generated.h"

class AEscapeChroniclesPlayerState;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEscapeChroniclesPlayerStart(const FObjectInitializer& ObjectInitializer);

	/**
	 * This must be called when choosing the PlayerStart for a pawn.
	 * @param PlayerState PlayerState of the pawn that is going to spawn at this PlayerStart.
	 * @return Whether the pawn can spawn at this PlayerStart or not.
	 */
	bool CanSpawnPawn(const AEscapeChroniclesPlayerState* PlayerState) const;

private:
	/**
	 * Required tags for pawn to spawn at this PlayerStart. If a pawn doesn't have any of these tags, it won't be able
	 * to spawn here.
	 */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer RequiredGameplayTagsToSpawn;
};