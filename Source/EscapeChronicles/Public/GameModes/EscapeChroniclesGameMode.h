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

private:
	FUniquePlayerIdManager UniquePlayerIdManager;

	bool bInitialGameLoadFinishedOrFailed = false;

	void OnInitialGameLoadFinishedOrFailed();

	TArray<TWeakObjectPtr<APlayerController>> PlayersWaitingToBeLoaded;

	void LoadPlayerNowOrWhenPawnIsPossessed(APlayerController* PlayerController) const;

	void OnPlayerToLoadPawnChanged(APawn* NewPawn) const;

	void LoadPlayerOrGenerateUniquePlayerIdForPlayer(const APlayerController* PlayerController) const;
};