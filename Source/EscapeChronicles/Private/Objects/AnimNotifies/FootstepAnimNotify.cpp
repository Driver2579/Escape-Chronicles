// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/AnimNotifies/FootstepAnimNotify.h"

#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

void UFootstepAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// === Determining if the character is on the ground by the trace downwards ===
	
	const FVector LineTraceStart = MeshComp->GetSocketLocation(FootSocketName);
	
	FVector LineTraceEnd = LineTraceStart;
	LineTraceEnd.Z -= LineTraceDepth;
	
	FCollisionQueryParams LineTraceParams;
	LineTraceParams.AddIgnoredActor(MeshComp->GetOwner());
	LineTraceParams.bReturnPhysicalMaterial = true;
	
	FHitResult HitResult;
	const bool bHit = MeshComp->GetWorld()->LineTraceSingleByChannel(HitResult, LineTraceStart, LineTraceEnd,
		ECC_Visibility, LineTraceParams);

	// === Play the sound if the ground is underfoot ===
	
	if (!bHit)
	{
		return;
	}

	// The sound we want to play
	USoundBase* Sound;

	// If the material underfoot is not suitable, we play a standard sound
	if (!HitResult.PhysMaterial.IsValid() || !SoundsByMaterial.Contains(HitResult.PhysMaterial.Get()))
	{
#if DO_ENSURE
		ensureAlways(DefaultSound);
#endif
		
		Sound = DefaultSound;
	}
	else
	{
		Sound = SoundsByMaterial[HitResult.PhysMaterial.Get()];
	}

	if (ensureAlways(IsValid(Sound)))
	{
		UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), Sound, HitResult.Location, VolumeMultiplier,
			PitchMultiplier);
	}
}
