// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MeleeHitDetectionAnimNotifyState.generated.h"

class AEscapeChroniclesPlayerState;

/**
 * AnimNotifyState for detecting melee attacks during animation playback. Performs sphere sweeps between animation
 * frames to detect hits and sends gameplay events.
 */
UCLASS(meta=(DisplayName="Punching Damage"))
class ESCAPECHRONICLES_API UMeleeHitDetectionAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
private:
	// Socket name to use for hit detection
	UPROPERTY(EditAnywhere)
	FName SocketName;

	// Radius of the sphere used for hit detection
	UPROPERTY(EditAnywhere)
	float Radius = 5.0f;

	// Interval between hit checks (in seconds)
	UPROPERTY(EditAnywhere)
	float TickInterval = 0.05f;

	// Gameplay tag to send when hit is detected
	UPROPERTY(EditAnywhere)
	FGameplayTag GameplayEventTag;

	// Collision channel to check for hits against
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Pawn;
	
#if WITH_EDITORONLY_DATA
	// Debug visualization toggle
	UPROPERTY(EditAnywhere)
	bool bDrawDebugSpheres = false;
#endif

	// Accumulated time since last hit check
	float ElapsedTime = 0.0f;

	// Previous frame's socket location for sweep detection
	FVector LastSocketLocation;

	// Flag indicating if we've already hit something
	bool bHit = false;

	FCollisionQueryParams CollisionParams;
	
	TWeakObjectPtr<APlayerState> InstigatorPlayerState = nullptr;

	// Performs a melee hit check from the socket location
	void CheckMeleeHit(const USkeletalMeshComponent* MeshComp);

	// Sends gameplay event to the target actor
	void SendGameplayEvent(AActor* Target) const;
};
