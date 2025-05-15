// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SendGameplayEventAnimNotify.generated.h"

class IAbilitySystemInterface;
class AEscapeChroniclesPlayerState;

/**
 *
 */
UCLASS(meta=(DisplayName="Melee Hit Detection"))
class ESCAPECHRONICLES_API USendGameplayEventAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
    
private:
	// Gameplay tag to send
	UPROPERTY(EditAnywhere)
	FGameplayTag GameplayEventTag;
};
