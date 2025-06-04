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
	// Name of the socket at whose location the sound will be played
	UPROPERTY(EditAnywhere)
	FName FootSocketName;

	// How far down will the trace go to check for the presence of ground
	UPROPERTY(EditAnywhere)
	int32 LineTraceDepth;

	// Sound that is played when there is no suitable value in SoundsByMaterial
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> DefaultSound;

	/**
	 * Map of surface material types to corresponding impact sounds. Used to play different sounds depending on the
	 * physical material an object interacts with.
	 * @tparam KeyType Physical material (e.g., dirt, metal, wood).
	 * @tparam ValueType Sound to play upon interaction with that material.
	 */
	UPROPERTY(EditAnywhere)
	TMap<TObjectPtr<UPhysicalMaterial>, TObjectPtr<USoundBase>> SoundsByMaterial;
	
	// Volume of the footstep sound
	UPROPERTY(EditAnywhere)
	float VolumeMultiplier = 1.0f;

	// Pitch of the footstep sound
	UPROPERTY(EditAnywhere)
	float PitchMultiplier = 1.0f;
};
