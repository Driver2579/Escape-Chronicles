// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/CarryCharacterGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Actors/ActivitySpot.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/CarryCharacterComponent.h"

void UCarryCharacterGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor);

	if (!ensureAlways(Character))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	UCarryCharacterComponent* CarryCharacterComponent = Character->GetCarryCharacterComponent();

	if (!ensureAlways(CarryCharacterComponent))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	AEscapeChroniclesCharacter* CarriedCharacter = CarryCharacterComponent->GetCarriedCharacter();

	FVector StartViewLocation;
	FRotator ViewRotation;
	ActorInfo->PlayerController->GetPlayerViewPoint(StartViewLocation, ViewRotation);

	const FVector EndViewLocation = StartViewLocation + ViewRotation.Vector() * CarryPickupDistance;

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(ActorInfo->AvatarActor.Get());
	if (IsValid(CarriedCharacter))
	{
		TraceParams.AddIgnoredActor(CarriedCharacter);
	}
	TraceParams.bTraceComplex = true;

	FHitResult OutHit;
	if (!GetWorld()->LineTraceSingleByChannel(OutHit, StartViewLocation, EndViewLocation, ECC_PhysicsBody, TraceParams))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);

		return;
	}

	if (IsValid(CarriedCharacter))
	{
		CarryCharacterComponent->SetCarriedCharacter(nullptr);

		AActivitySpot* ActivitySpotToDrop = Cast<AActivitySpot>(OutHit.GetActor());

		if (IsValid(ActivitySpotToDrop) && ActivitySpotToDrop->IsA(ActivitySpotToDropClass))
		{
			ActivitySpotToDrop->SetOccupyingCharacter(CarriedCharacter);
		}
	}
	else
	{
		AEscapeChroniclesCharacter* CharacterToCarry = Cast<AEscapeChroniclesCharacter>(OutHit.GetActor());

		if (IsValid(CharacterToCarry))
		{
			CarryCharacterComponent->SetCarriedCharacter(CharacterToCarry);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
