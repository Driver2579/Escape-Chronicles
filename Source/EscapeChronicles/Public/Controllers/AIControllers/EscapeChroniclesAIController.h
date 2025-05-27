// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "EscapeChroniclesAIController.generated.h"

class UStateTreeAIComponent;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()

public:
	AEscapeChroniclesAIController();

	virtual void InitPlayerState() override;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI", meta=(AllowPrivateAccess ="true"))
	UStateTreeAIComponent* StateTreeAIComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AEscapeChroniclesPlayerState> PlayerStateClassOverride;
};