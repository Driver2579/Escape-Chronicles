// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/ActivitySpot.h"
#include "Bed.generated.h"

class UPlayerOwnershipComponent;

// Works with UBedsManagementSubsystem
UCLASS()
class ESCAPECHRONICLES_API ABed : public AActivitySpot
{
	GENERATED_BODY()

public:
	ABed();
	
protected:
	
	virtual void OccupySpot(AEscapeChroniclesCharacter* Character) override;
	virtual void UnoccupySpot(AEscapeChroniclesCharacter* Character) override;

private:
	UPROPERTY(EditAnywhere, SaveGame)
	TObjectPtr<UPlayerOwnershipComponent> PlayerOwnershipComponent;
};
