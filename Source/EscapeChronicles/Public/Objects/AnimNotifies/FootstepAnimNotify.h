// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FootstepAnimNotify.generated.h"

// Does a trace down from the socket and plays a sound depending on the material the character is stepping on
UCLASS()
class ESCAPECHRONICLES_API UFootstepAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	//  Socket name to play sound location to
	UPROPERTY(EditAnywhere)
	FName FootSocketName;

	// The depth to which the trace is lowered
	UPROPERTY(EditAnywhere)
	int32 LineTraceDepth;
	
	UPROPERTY(EditAnywhere)
	TMap<TObjectPtr<UPhysicalMaterial>, TObjectPtr<USoundBase>> SoundsByMaterial;
	
	// Socket or bone name to attach sound to
	UPROPERTY(EditAnywhere)
	float VolumeMultiplier = 1.0f;

	// Socket or bone name to attach sound to
	UPROPERTY(EditAnywhere)
	float PitchMultiplier = 1.0f;
};
