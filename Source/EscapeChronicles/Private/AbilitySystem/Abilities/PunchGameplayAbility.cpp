// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PunchGameplayAbility.h"

#include "AbilitySystemGlobals.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

void UPunchGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);

		return;
	}
	
	if (!ensureAlways(FMath::IsWithin(LastPlayedMontageIndex, 0, MontagesQueue.Num())))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		
		return;
	}

	LoadDamagingGameplayEffects();
	
	if (LastPlayedMontageIndex == MontagesQueue.Num() - 1)
	{
		LoadAndPlayAnimMontage(0);
	}
	else
	{
		LoadAndPlayAnimMontage(LastPlayedMontageIndex + 1);
	}
	
	// === Wait gameplay event ===

	const FGameplayEventTagMulticastDelegate::FDelegate OnWaitGameplayEventDelegate =
		FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnWaitGameplayEventSent);

	WaitGameplayEventDelegateHandle = OnWaitGameplayEventDelegate.GetHandle();
	
	CurrentActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(WaitGameplayEventTag), OnWaitGameplayEventDelegate);
}

void UPunchGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(
			FGameplayTagContainer(WaitGameplayEventTag), WaitGameplayEventDelegateHandle);
	}

	if (LoadCurrentAnimMontageHandle.IsValid())
	{
		LoadCurrentAnimMontageHandle->CancelHandle();
		LoadCurrentAnimMontageHandle.Reset();
	}
	
	if (LoadSuccessfulDamagingEffectHandle.IsValid())
	{
		LoadSuccessfulDamagingEffectHandle->CancelHandle();
		LoadSuccessfulDamagingEffectHandle.Reset();
	}

	if (LoadUnsuccessfulDamagingEffectHandle.IsValid())
	{
		LoadUnsuccessfulDamagingEffectHandle->CancelHandle();
		LoadUnsuccessfulDamagingEffectHandle.Reset();
	}

	InstigatorAbilitySystemComponent = nullptr;
	TargetAbilitySystemComponent = nullptr;
	DesiredToApplyGameplayEffectClass.Reset();
	WasEventCalledBeforeEffectLoaded = false;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPunchGameplayAbility::LoadAndPlayAnimMontage(const int32 Index)
{
	const TSoftObjectPtr<UAnimMontage>& NeededMontage = MontagesQueue[Index];
	
	if (!ensureAlways(!NeededMontage.IsNull()))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}

	LoadCurrentAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		NeededMontage.ToSoftObjectPath(), 
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnAnimMontageLoaded, Index));
}

void UPunchGameplayAbility::OnAnimMontageLoaded(const int32 Index)
{
	const TSoftObjectPtr<UAnimMontage>& AnimMontage = MontagesQueue[Index];
	
#if DO_CHECK
	check(CurrentActorInfo);
	check(CurrentActorInfo->AbilitySystemComponent.IsValid());
	check(AnimMontage.IsValid());
#endif
	
#if DO_ENSURE
	ensureAlways(WaitGameplayEventTag.IsValid());
#endif

	// === Play the animation ===
	
	CurrentActorInfo->AbilitySystemComponent->PlayMontage(this, CurrentActivationInfo, AnimMontage.Get(), 1);
	
	FOnMontageBlendingOutStarted OnAnimMontageBlendingOutDelegate =
		FOnMontageBlendingOutStarted::CreateLambda([this](UAnimMontage* AnimMontage, bool bInterrupted)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		});

	CurrentActorInfo->GetAnimInstance()->Montage_SetBlendingOutDelegate(OnAnimMontageBlendingOutDelegate,
		AnimMontage.Get());

	LastPlayedMontageIndex = Index;
}

void UPunchGameplayAbility::LoadDamagingGameplayEffects()
{
	if (!ensureAlways(!SuccessfulDamagingEffectClass.IsNull() && !UnsuccessfulDamagingEffectClass.IsNull()))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}
	
	LoadSuccessfulDamagingEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SuccessfulDamagingEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this]
		{
			if (WasEventCalledBeforeEffectLoaded && DesiredToApplyGameplayEffectClass == SuccessfulDamagingEffectClass)
			{
				ApplyDesiredGameplayEffect();
			}
		}));

	LoadUnsuccessfulDamagingEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		UnsuccessfulDamagingEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this]
		{
			if (WasEventCalledBeforeEffectLoaded && DesiredToApplyGameplayEffectClass == UnsuccessfulDamagingEffectClass)
			{
				ApplyDesiredGameplayEffect();
			}
		}));
}

void UPunchGameplayAbility::OnWaitGameplayEventSent(FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData)
{
	InstigatorAbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GameplayEventData->Instigator);
	
	TargetAbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GameplayEventData->Target);
	
	if (!IsValid(InstigatorAbilitySystemComponent) || !IsValid(TargetAbilitySystemComponent))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}

	// Select the desired effect
	if (TargetAbilitySystemComponent->HasMatchingGameplayTag(BlockingPunchesTag))
	{
		DesiredToApplyGameplayEffectClass = SuccessfulDamagingEffectClass;
	}
	else
	{
		DesiredToApplyGameplayEffectClass = UnsuccessfulDamagingEffectClass;
	}

	if (!DesiredToApplyGameplayEffectClass.IsValid())
	{
		WasEventCalledBeforeEffectLoaded = true;
		
		return;
	}
	
	ApplyDesiredGameplayEffect();
}

void UPunchGameplayAbility::ApplyDesiredGameplayEffect()
{
	if (!IsValid(InstigatorAbilitySystemComponent) || !IsValid(TargetAbilitySystemComponent))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}
	
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetAbilitySystemComponent->MakeOutgoingSpec(
				DesiredToApplyGameplayEffectClass.Get(), GetAbilityLevel(), FGameplayEffectContextHandle());
				
	InstigatorAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),
		TargetAbilitySystemComponent);
}

