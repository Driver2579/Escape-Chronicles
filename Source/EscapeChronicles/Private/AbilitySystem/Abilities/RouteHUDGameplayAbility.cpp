// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/RouteHUDGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "HUDs/EscapeChroniclesHUD.h"

URouteHUDGameplayAbility::URouteHUDGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void URouteHUDGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (!ensureAlways(IsValid(Character)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	const APlayerController* PlayerController = Character->GetController<APlayerController>();

	if (!ensureAlways(IsValid(PlayerController)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	AEscapeChroniclesHUD* HUD = PlayerController->GetHUD<AEscapeChroniclesHUD>();
	
	if (!ensureAlways(IsValid(HUD)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	HUD->GoTo(RouteTag);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
