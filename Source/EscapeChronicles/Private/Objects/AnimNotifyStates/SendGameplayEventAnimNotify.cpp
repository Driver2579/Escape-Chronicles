// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/Objects/AnimNotifies/SendGameplayEventAnimNotify.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

void USendGameplayEventAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UAbilitySystemComponent* InstigatorAbilitySystemComponent
		= UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MeshComp->GetOwner());
    
	if (IsValid(InstigatorAbilitySystemComponent))
	{
		FGameplayEventData GameplayEventData;
		GameplayEventData.Instigator = MeshComp->GetOwner();
		InstigatorAbilitySystemComponent->HandleGameplayEvent(GameplayEventTag, &GameplayEventData);
	}
}
