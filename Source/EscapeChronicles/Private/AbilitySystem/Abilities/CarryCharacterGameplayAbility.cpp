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
	

	// Calculate view trace parameters
	FVector StartViewLocation;
	FRotator ViewRotation;
	ActorInfo->PlayerController->GetPlayerViewPoint(StartViewLocation, ViewRotation);

	const FVector EndViewLocation = StartViewLocation + ViewRotation.Vector() * CarryPickupDistance;

	// Set up trace parameters
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(ActorInfo->AvatarActor.Get());

	// Check if we're currently carrying someone
	AEscapeChroniclesCharacter* CarriedCharacter = CarryCharacterComponent->GetCarriedCharacter();
	if (IsValid(CarriedCharacter))
	{
		TraceParams.AddIgnoredActor(CarriedCharacter);
	}
	TraceParams.bTraceComplex = true;

	// Perform raycast
	FHitResult OutHit;
	if (!GetWorld()->LineTraceSingleByChannel(OutHit, StartViewLocation, EndViewLocation, ECC_PhysicsBody, TraceParams))
	{
		// If nothing was hit, end the ability

		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);

		return;
	}

	// Handle interaction based on whether we're carrying someone
	if (IsValid(CarriedCharacter))
	{
		// Drop currently carried character
		CarryCharacterComponent->SetCarriedCharacter(nullptr);

		// Check if we hit a valid activity spot
		AActivitySpot* ActivitySpotToDrop = Cast<AActivitySpot>(OutHit.GetActor());
		if (IsValid(ActivitySpotToDrop) && ActivitySpotToDrop->IsA(ActivitySpotToDropClass))
		{
			ActivitySpotToDrop->SetOccupyingCharacter(CarriedCharacter);
		}
	}
	else
	{
		// Try to pick up a character
		AEscapeChroniclesCharacter* CharacterToCarry = Cast<AEscapeChroniclesCharacter>(OutHit.GetActor());
		if (IsValid(CharacterToCarry))
		{
			CarryCharacterComponent->SetCarriedCharacter(CharacterToCarry);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
