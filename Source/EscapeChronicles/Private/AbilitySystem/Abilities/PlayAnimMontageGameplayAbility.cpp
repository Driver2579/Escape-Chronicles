// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PlayAnimMontageGameplayAbility.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

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
	
	const AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(
		ActorInfo->AvatarActor.Get(), ECastCheckedType::NullAllowed);
	
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
	
	if (!ensureAlways(IsValid(Character)))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, false);
		
		return;
	}
	
	switch (PlayingMethod)
	{
	case EPlayingMethod::Queue: PlayQueueMontage(); break;
	case EPlayingMethod::Random: PlayRandomMontage(); break;
	}
	
	//EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UPlayAnimMontageGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	/**
	 * Unregister the event only if AbilitySystemComponent is still valid, because it could be already destroyed at this
	 * moment.
	 */
	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->GenericGameplayEventCallbacks.Remove(GameplayEventTag);
	}

	// Cancel loading of the AnimMontage if it's still loading or unload it if it's loaded
	if (LoadCurrentAnimMontageHandle.IsValid())
	{
		LoadCurrentAnimMontageHandle->CancelHandle();
		LoadCurrentAnimMontageHandle.Reset();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPlayAnimMontageGameplayAbility::PlayRandomMontage()
{
	if (!ensureAlways(AvailableMontages.Num() >= 0))
	{
		return;
	}
	
	const int32 RandomIndex = FMath::RandRange(0, AvailableMontages.Num() - 1);
	
	LoadAndPlayAnimMontage(RandomIndex);
}

void UPlayAnimMontageGameplayAbility::PlayQueueMontage()
{
	if (!ensureAlways(FMath::IsWithin(LastPlayedMontageIndex, 0, AvailableMontages.Num())))
	{
		return;
	}

	if (LastPlayedMontageIndex == AvailableMontages.Num() - 1)
	{
		LoadAndPlayAnimMontage(0);
	}
	else
	{
		LoadAndPlayAnimMontage(LastPlayedMontageIndex + 1);
	}
}

void UPlayAnimMontageGameplayAbility::LoadAndPlayAnimMontage(const int32 Index)
{
	if (!FMath::IsWithin(Index, 0, AvailableMontages.Num()))
	{
		return;
	}
	
	const TSoftObjectPtr<UAnimMontage>& NeededMontage = AvailableMontages[Index];
	
	if (ensureAlways(!NeededMontage.IsNull()))
	{      
		LoadCurrentAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			NeededMontage.ToSoftObjectPath(), 
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnAnimMontageLoaded, Index));
	}
}

void UPlayAnimMontageGameplayAbility::OnAnimMontageLoaded(const int32 Index)
{
	const TSoftObjectPtr<UAnimMontage>& AnimMontage = AvailableMontages[Index];
	
#if DO_CHECK
	check(CurrentActorInfo);
	check(CurrentActorInfo->AbilitySystemComponent.IsValid());
	check(AnimMontage.IsValid());
#endif
	
#if DO_ENSURE
	ensureAlways(GameplayEventTag.IsValid());
#endif

	// Get the event's delegate
	//FGameplayEventMulticastDelegate& GameplayEventReceivedDelegate =
	//	CurrentActorInfo->AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(GameplayEventTag);
	
	/*// Bind the event's delegate
	GameplayEventReceivedDelegate.AddLambda([](const FGameplayEventData* GameplayEventData)
	{
		UE_LOG(LogTemp, Error, TEXT("Ахуеть, работает!"));
	});*/
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		GameplayEventTag
	);

	WaitEventTask->EventReceived.AddDynamic(this, &UPlayAnimMontageGameplayAbility::Test);
	//WaitEventTask->ReadyForActivation();
	WaitEventTask->Activate();
	
	// Play the animation
	CurrentActorInfo->AbilitySystemComponent->PlayMontage(this, CurrentActivationInfo, AnimMontage.Get(), 1);
	
	// === Listen if animation was interrupted by something ===

	FOnMontageBlendingOutStarted OnAnimMontageBlendingOutDelegate =
		FOnMontageBlendingOutStarted::CreateUObject(this, &ThisClass::OnDeployBombAnimMontageBlendingOut);

	CurrentActorInfo->GetAnimInstance()->Montage_SetBlendingOutDelegate(OnAnimMontageBlendingOutDelegate, AnimMontage.Get());

	// === Listen for animation end ===

	FOnMontageEnded OnAnimMontageEndedDelegate = FOnMontageEnded::CreateUObject(this,
		&ThisClass::OnAnimMontageEnded);

	CurrentActorInfo->GetAnimInstance()->Montage_SetEndDelegate(OnAnimMontageEndedDelegate, AnimMontage.Get());

	LastPlayedMontageIndex = Index;
}

void UPlayAnimMontageGameplayAbility::OnDeployBombAnimMontageBlendingOut(UAnimMontage* AnimMontage, bool bInterrupted)
{
	UE_LOG(LogTemp, Error, TEXT("OnDeployBombAnimMontageBlendingOut!"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPlayAnimMontageGameplayAbility::OnAnimMontageEnded(UAnimMontage* AnimMontage, bool bInterrupted)
{
	UE_LOG(LogTemp, Error, TEXT("OnAnimMontageEnded!"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPlayAnimMontageGameplayAbility::Test(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Error, TEXT("Ахуеть, работает!"));
}
