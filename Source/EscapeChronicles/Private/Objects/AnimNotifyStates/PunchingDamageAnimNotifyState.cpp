// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/AnimNotifyStates/PunchingDamageAnimNotifyState.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/OverlapResult.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UPunchingDamageAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ElapsedTime = 0;
	bIsTargetFound = false;
	
	TryToDamage(MeshComp);
}

void UPunchingDamageAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (TickInterval <= 0.0f)
	{
		TryToDamage(MeshComp);
		
		return;
	}
	
	ElapsedTime += FrameDeltaTime;

	while (ElapsedTime >= TickInterval)
	{
		TryToDamage(MeshComp);
		
		ElapsedTime -= TickInterval;
	}
}

void UPunchingDamageAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	TryToDamage(MeshComp);
}

void UPunchingDamageAnimNotifyState::TryToDamage(const USkeletalMeshComponent* MeshComp)
{
	if (bIsTargetFound)
	{
		return;
	}
	
	if (!ensureAlways(IsValid(MeshComp)))
	{
		return;
	}

	const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);

#if WITH_EDITOR
	if (bDrawDebugSpheres)
	{
		DrawDebugSphere(MeshComp->GetWorld(), SocketLocation, Radius, 50, FColor::Red, false, 1);
	}
#endif
	
	AActor* Owner = MeshComp->GetOwner();

	if (!ensureAlways(IsValid(Owner)))
	{
		return;
	}
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Owner);
	
	FHitResult HitResult;
	/*
	const bool bHit = MeshComp->GetWorld()->SweepSingleByChannel(HitResult, SocketLocation, SocketLocation,
		FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(Radius), CollisionParams);
		*/
	
	TArray<FOverlapResult> OverlappedActors;
	bool bHit = MeshComp->GetWorld()->OverlapMultiByObjectType(
		OverlappedActors,
		SocketLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius),
		CollisionParams
	);
	
	if (!bHit)
	{
		return;
	}
	
	const AEscapeChroniclesCharacter* TargetCharacter = nullptr; // = Cast<AEscapeChroniclesCharacter>(HitResult.GetActor());

	for (FOverlapResult& OverlappedActor : OverlappedActors)
	{
		const AEscapeChroniclesCharacter* Casted = Cast<AEscapeChroniclesCharacter>(OverlappedActor.GetActor());

		if (IsValid(Casted))
		{
			TargetCharacter = Casted;
		}
	}
	
	if (!IsValid(TargetCharacter))
	{
		return;
	}

	const AEscapeChroniclesPlayerState* TargetPlayerState = TargetCharacter->GetPlayerState<AEscapeChroniclesPlayerState>();

	if (!IsValid(TargetPlayerState))
	{
		return;
	}

	UAbilitySystemComponent* TargetAbilitySystemComponent = TargetPlayerState->GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(TargetAbilitySystemComponent)))
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = TargetAbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddInstigator(Owner, Owner);

	const FGameplayEffectSpecHandle SpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(DamagingEffectClass,
		DamagingEffectLevel, ContextHandle);
	
	TargetAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetAbilitySystemComponent);

	bIsTargetFound = true;
}