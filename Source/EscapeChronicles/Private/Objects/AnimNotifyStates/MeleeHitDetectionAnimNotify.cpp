// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/AnimNotifyStates/MeleeHitDetectionAnimNotify.h"

#include "AbilitySystemGlobals.h"
#include "Engine/OverlapResult.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UMeleeHitDetectionAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);

#if WITH_EDITOR
	if (bDrawDebug)
	{
		// Draw the location of hit
		DrawDebugSphere(MeshComp->GetWorld(), SocketLocation, Radius, 1, FColor::Blue, false, 1.0f);
	}
#endif
    
	// === Perform sphere sweep between current and previous socket locations ===

	FCollisionQueryParams CollisionParams;
	FCollisionObjectQueryParams CollisionObjectQueryParams;

	CollisionParams.AddIgnoredActor(MeshComp->GetOwner());
    
	for (const ECollisionChannel CollisionChannel : CollisionChannels)
	{
		CollisionObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);
	}
    
	TArray<FOverlapResult> OverlapResults;
	const bool bHit = MeshComp->GetWorld()->OverlapMultiByObjectType(OverlapResults, SocketLocation,
		FQuat::Identity, CollisionObjectQueryParams, FCollisionShape::MakeSphere(Radius), CollisionParams);
    
	if (bHit)
	{
		SendGameplayEvent(MeshComp->GetOwner(), OverlapResults[0].GetActor());
	}
}

void UMeleeHitDetectionAnimNotify::SendGameplayEvent(AActor* Instigator, AActor* Target) const
{
	if (!IsValid(Instigator) || !IsValid(Target))
	{
		return;
	}
    
	UAbilitySystemComponent* InstigatorAbilitySystemComponent
		= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Instigator);
    
	if (!IsValid(InstigatorAbilitySystemComponent))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.Instigator = Instigator;
	EventData.Target = Target;
    
	InstigatorAbilitySystemComponent->HandleGameplayEvent(GameplayEventTag, &EventData);
}
