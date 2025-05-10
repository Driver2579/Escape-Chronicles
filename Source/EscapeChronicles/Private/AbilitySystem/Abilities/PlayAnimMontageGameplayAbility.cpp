// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PlayAnimMontageGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

UPlayAnimMontageGameplayAbility::UPlayAnimMontageGameplayAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UPlayAnimMontageGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}

	const AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(ActorInfo->AvatarActor.Get(),
		ECastCheckedType::NullAllowed);
	
	if (!ensureAlways(IsValid(Character)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		
		return;
	}

	const USkeletalMeshComponent* Mesh = Character->GetMesh();

	if (!ensureAlways(IsValid(Mesh)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		
		return;
	}
	
	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	
	if (!ensureAlways(IsValid(Character)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		
		return;
	}
	
	switch (PlayingMethod)
	{
	case EPlayingMethod::Queue: PlayQueueMontage(AnimInstance); break;
	case EPlayingMethod::Random: PlayRandomMontage(AnimInstance); break;
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UPlayAnimMontageGameplayAbility::PlayRandomMontage(UAnimInstance* AnimInstance)
{
	if (!ensureAlways(AvailableMontages.Num() >= 0))
	{
		return;
	}
	
	const int32 RandomIndex = FMath::RandRange(0, AvailableMontages.Num() - 1);
	
	AnimInstance->Montage_Play(AvailableMontages[RandomIndex]);

	LastPlayedMontage = RandomIndex;
}

void UPlayAnimMontageGameplayAbility::PlayQueueMontage(UAnimInstance* AnimInstance)
{
	if (!ensureAlways(FMath::IsWithin(LastPlayedMontage, 0, AvailableMontages.Num())))
	{
		return;
	}

	AnimInstance->Montage_Play(AvailableMontages[LastPlayedMontage]);

	if (LastPlayedMontage == AvailableMontages.Num() - 1)
	{
		LastPlayedMontage = 0;
	}
	else
	{
		LastPlayedMontage += 1;
	}
}
