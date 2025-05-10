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
	bIsTargetDamaged = false;
	StateOwner = MeshComp->GetOwner();
	
	FindAndDamageTarget(MeshComp);
}

void UPunchingDamageAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (TickInterval <= 0.0f)
	{
		FindAndDamageTarget(MeshComp);
		
		return;
	}
	
	ElapsedTime += FrameDeltaTime;

	while (ElapsedTime >= TickInterval)
	{
		FindAndDamageTarget(MeshComp);
		
		ElapsedTime -= TickInterval;
	}
}

void UPunchingDamageAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	FindAndDamageTarget(MeshComp);
}

void UPunchingDamageAnimNotifyState::FindAndDamageTarget(const USkeletalMeshComponent* MeshComp)
{
	if (bIsTargetDamaged)
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
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(StateOwner.Get());
	
	TArray<FOverlapResult> OverlappedActors;
	const bool bOverlapped = MeshComp->GetWorld()->OverlapMultiByObjectType(OverlappedActors, SocketLocation,
		FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Radius),
		CollisionParams);
	
	if (!bOverlapped)
	{
		return;
	}
	
	const AEscapeChroniclesCharacter* TargetCharacter = nullptr;

	for (FOverlapResult& OverlappedActor : OverlappedActors)
	{
		const AEscapeChroniclesCharacter* CastedCharacter = Cast<AEscapeChroniclesCharacter>(
			OverlappedActor.GetActor());

		if (IsValid(CastedCharacter))
		{
			TargetCharacter = CastedCharacter;
		}
	}
	
	DamageTarget(TargetCharacter);
}

void UPunchingDamageAnimNotifyState::DamageTarget(const AEscapeChroniclesCharacter* TargetCharacter)
{
	if (!TargetCharacter)
	{
		return;
	}
	
	const AEscapeChroniclesPlayerState* TargetPlayerState =
		TargetCharacter->GetPlayerState<AEscapeChroniclesPlayerState>();

	if (!IsValid(TargetPlayerState))
	{
		return;
	}

	UAbilitySystemComponent* TargetAbilitySystemComponent = TargetPlayerState->GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(TargetAbilitySystemComponent)))
	{
		return;
	}

	// === Application of gameplay effect ===
	
	FGameplayEffectContextHandle ContextHandle = TargetAbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddInstigator(StateOwner.Get(), StateOwner.Get());

	const FGameplayEffectSpecHandle SpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(DamagingEffectClass,
		DamagingEffectLevel, ContextHandle);
	
	TargetAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetAbilitySystemComponent);

	bIsTargetDamaged = true;
}
