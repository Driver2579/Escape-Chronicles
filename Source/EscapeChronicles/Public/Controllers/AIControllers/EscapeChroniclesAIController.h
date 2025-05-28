// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "EscapeChroniclesAIController.generated.h"

class AEscapeChroniclesPlayerState;
class UStateTreeAIComponent;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()

public:
	AEscapeChroniclesAIController();

	virtual void PostInitializeComponents() override;

	virtual void InitPlayerState() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess ="true"))
	UStateTreeAIComponent* StateTreeAIComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AEscapeChroniclesPlayerState> PlayerStateClassOverride;

	void OnBotInitialized(AEscapeChroniclesPlayerState* InitializedPlayerState) const;
};