// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MeleeHitDetectionAnimNotify.generated.h"

class IAbilitySystemInterface;
class AEscapeChroniclesPlayerState;

/**
 * AnimNotify for detecting melee attacks during animation playback. Performs sphere sweeps between animation
 * frames to detect hits and sends gameplay events.
 */
UCLASS(meta=(DisplayName="Melee Hit Detection"))
class ESCAPECHRONICLES_API UMeleeHitDetectionAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
    
private:
	// Socket name to use for hit detection
	UPROPERTY(EditAnywhere)
	FName SocketName;

	// Radius of the sphere used for hit detection
	UPROPERTY(EditAnywhere)
	float Radius = 5.0f;

	// Gameplay tag to send when hit is detected
	UPROPERTY(EditAnywhere)
	FGameplayTag GameplayEventTag;
    
	// Collision channel to check for hits against
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels;
    
#if WITH_EDITORONLY_DATA
	// Debug visualization toggle
	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;
#endif

	// Sends gameplay event to the instigator actor
	void SendGameplayEvent(AActor* Instigator, AActor* Target) const;
};
