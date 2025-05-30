// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Saveable.h"
#include "PrisonerChamberZone.generated.h"

class UBoxComponent;
class UPlayerOwnershipComponent;

UCLASS()
class ESCAPECHRONICLES_API APrisonerChamberZone : public AActor, public ISaveable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APrisonerChamberZone();

	UBoxComponent* GetOuterZoneBoxComponent() const { return OuterZoneBoxComponent; }
	UBoxComponent* GetInnerZoneBoxComponent() const { return InnerZoneBoxComponent; }

	UPlayerOwnershipComponent* GetPlayerOwnershipComponent() const { return PlayerOwnershipComponent; }

private:
	// A zone the player needs to stop overlapping with to stop being considered he is in the chamber
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> OuterZoneBoxComponent;

	/**
	 * A zone the player needs to overlap with to start being considered he is in the chamber. This trigger should be
	 * fully inside the Outer Zone trigger.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InnerZoneBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlayerOwnershipComponent> PlayerOwnershipComponent;
};