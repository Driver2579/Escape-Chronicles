// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/AnimNotifies/FootstepAnimNotify.h"

#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

void UFootstepAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	const FVector LineTraceStart = MeshComp->GetSocketLocation(FootSocketName);
	FVector LineTraceEnd = LineTraceStart;
	
	LineTraceEnd.Z -= LineTraceDepth;
	
	FCollisionQueryParams LineTraceParams;
	LineTraceParams.AddIgnoredActor(MeshComp->GetOwner());
	LineTraceParams.bReturnPhysicalMaterial = true;
	
	FHitResult HitResult;
	const bool bHit = MeshComp->GetWorld()->LineTraceSingleByChannel(HitResult, LineTraceStart, LineTraceEnd,
		ECC_Visibility, LineTraceParams);
	
	if (!bHit || !ensure(HitResult.PhysMaterial.IsValid() && SoundsByMaterial.Contains(HitResult.PhysMaterial.Get())))
	{
		return;
	}
	
	USoundBase* Sound = SoundsByMaterial[HitResult.PhysMaterial.Get()];

	if (!ensure(IsValid(Sound)))
	{
		return;
	}
	
	UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), Sound, HitResult.Location, VolumeMultiplier,
		PitchMultiplier);
}
