// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/AnimNotifyStates/MeleeHitDetectionAnimNotifyState.h"

#include "AbilitySystemGlobals.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UMeleeHitDetectionAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// === Reset state for new activation ===
	
	ElapsedTime = 0;
	bHit = false;
	LastSocketLocation = MeshComp->GetSocketLocation(SocketName);

	CollisionParams.ClearIgnoredSourceObjects();
	CollisionParams.AddIgnoredActor(MeshComp->GetOwner());
	
	const APawn* Owner = MeshComp->GetOwner<APawn>();

	// Cache instigator's player state for damage attribution
	if (IsValid(Owner))
	{
		InstigatorPlayerState = Cast<AEscapeChroniclesPlayerState>(Owner->GetPlayerState());
	}
}

void UMeleeHitDetectionAnimNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	// If interval is 0, check every frame
	if (TickInterval <= 0.0f)
	{
		CheckMeleeHit(MeshComp);
		
		return;
	}

	// === Accumulate time and perform checks at specified intervals ===
	
	ElapsedTime += FrameDeltaTime;

	while (ElapsedTime >= TickInterval)
	{
		CheckMeleeHit(MeshComp);
		
		ElapsedTime -= TickInterval;
	}
}

void UMeleeHitDetectionAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// Final hit check when animation ends
	CheckMeleeHit(MeshComp);
}

void UMeleeHitDetectionAnimNotifyState::CheckMeleeHit(const USkeletalMeshComponent* MeshComp)
{
	// Skip if we've already hit something
	if (bHit)
	{
		return;
	}

	const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);

#if WITH_EDITOR
	// Debug visualization
	if (bDrawDebugSpheres)
	{
		const FVector LocationDelta = LastSocketLocation - SocketLocation;
		
		DrawDebugCapsule( MeshComp->GetWorld(), LocationDelta * 0.5f + SocketLocation,
			LocationDelta.Length() * 0.5f + Radius, Radius, FRotationMatrix::MakeFromZ(LocationDelta).ToQuat(),
			FColor::Red, false, 1.0f);
	}
#endif

	// === Perform sphere sweep between current and previous socket locations ===
	
	FHitResult HitResult;
	bHit = MeshComp->GetWorld()->SweepSingleByObjectType(HitResult, SocketLocation, LastSocketLocation,
		FQuat::Identity, FCollisionObjectQueryParams(CollisionChannel), FCollisionShape::MakeSphere(Radius),
		CollisionParams);

	LastSocketLocation = SocketLocation;
	
	if (bHit)
	{
		SendGameplayEvent(HitResult.GetActor());
	}
}

void UMeleeHitDetectionAnimNotifyState::SendGameplayEvent(AActor* Target) const
{
	if (!ensureAlways(InstigatorPlayerState.IsValid()))
	{
		return;
	}
		
	FGameplayEventData EventData;
	EventData.Instigator = InstigatorPlayerState.Get();

	// === Set target if it has an AbilitySystemComponent ===
	
	const APawn* TargetPawn = Cast<APawn>(Target);

	if (IsValid(TargetPawn))
	{
		APlayerState* TargetPlayerState = TargetPawn->GetPlayerState();

		if (Cast<IAbilitySystemInterface>(TargetPlayerState))
		{
			EventData.Target = TargetPlayerState;
		}
	}

	// === Send the gameplay event through the instigator's ability system ===
	
	UAbilitySystemComponent* InstigatorAbilitySystemComponent
		= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstigatorPlayerState.Get());

	if (ensureAlways(IsValid(InstigatorAbilitySystemComponent)))
	{
		InstigatorAbilitySystemComponent->HandleGameplayEvent(GameplayEventTag, &EventData);
	}
}
