// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EscapeChroniclesGameMode.generated.h"

UCLASS(MinimalAPI)
class AEscapeChroniclesGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEscapeChroniclesGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
		const FString& Options, const FString& Portal = L"") override;

	virtual void Logout(AController* Exiting) override;

private:
	void OnInitialGameLoadFinished()
	{
		bInitialGameLoadFinished = true;
	}

	bool bInitialGameLoadFinished = false;
};