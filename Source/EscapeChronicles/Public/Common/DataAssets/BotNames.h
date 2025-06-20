// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BotNames.generated.h"

// Non-mutable data asset that contains the names for the bots
UCLASS()
class ESCAPECHRONICLES_API UBotNames : public UDataAsset
{
	GENERATED_BODY()

public:
	const TSet<FString>& GetBotNames() const { return BotNames; }

private:
	UPROPERTY(EditAnywhere)
	TSet<FString> BotNames;
};