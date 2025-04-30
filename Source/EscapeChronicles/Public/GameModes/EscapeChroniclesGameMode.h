// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/UniquePlayerID.h"
#include "EscapeChroniclesGameMode.generated.h"

UCLASS(MinimalAPI)
class AEscapeChroniclesGameMode : public AGameModeBase, public ISaveable
{
	GENERATED_BODY()

public:
	AEscapeChroniclesGameMode();

	FUniquePlayerIdManager& GetUniquePlayerIdManager() { return UniquePlayerIdManager; }

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal = L"") override;

	virtual void Logout(AController* Exiting) override;

private:
	FUniquePlayerIdManager UniquePlayerIdManager;

	void OnInitialGameLoadFinishedOrFailed()
	{
		bInitialGameLoadFinishedOrFailed = true;
	}

	bool bInitialGameLoadFinishedOrFailed = false;

	UFUNCTION()
	void OnPlayerToLoadPawnChanged(APawn* OldPawn, APawn* NewPawn);
};