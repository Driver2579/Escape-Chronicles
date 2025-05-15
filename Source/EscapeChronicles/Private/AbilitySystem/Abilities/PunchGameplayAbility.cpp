// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PunchGameplayAbility.h"

#include "AbilitySystemGlobals.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/SphereComponent.h"
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
	
	if (LastPlayedMontageIndex == MontagesQueue.Num() - 1)
	{
		LoadAndPlayAnimMontage(0);
	}
	else
	{
		LoadAndPlayAnimMontage(LastPlayedMontageIndex + 1);
	}

	if (!HasAuthority(&ActivationInfo))
	{
		return;
	}
	
	LoadGameplayEffects();
	
	const FGameplayEventTagMulticastDelegate::FDelegate OnStartDamageFrameEventTagDelegate =
		FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnStartDamageFrame);
	
	CurrentActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(StartDamageFrameEventTag), OnStartDamageFrameEventTagDelegate);

	OnStartDamageFrameEventTagDelegateHandle = OnStartDamageFrameEventTagDelegate.GetHandle();
}

void UPunchGameplayAbility::LoadAndPlayAnimMontage(const int32 Index)
{
	if (!ensureAlways(IsValid(CurrentActorInfo->AvatarActor.Get())))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		
		return;
	}
	
	MeleeHitComponent = CurrentActorInfo->AvatarActor->FindComponentByTag<UShapeComponent>(
		MontagesQueue[Index].DamagingComponentTag);
	
	const TSoftObjectPtr<UAnimMontage>& DesiredMontage = MontagesQueue[Index].AnimMontage;
	
	if (!ensureAlways(!DesiredMontage.IsNull()))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}

	LoadCurrentAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		DesiredMontage.ToSoftObjectPath(), 
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnAnimMontageLoaded, Index));
}

void UPunchGameplayAbility::OnAnimMontageLoaded(const int32 Index)
{
	const TSoftObjectPtr<UAnimMontage>& AnimMontage = MontagesQueue[Index].AnimMontage;
	
#if DO_CHECK
	check(CurrentActorInfo);
	check(CurrentActorInfo->AbilitySystemComponent.IsValid());
	check(AnimMontage.IsValid());
#endif
	
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

void UPunchGameplayAbility::LoadGameplayEffects()
{
	if (!ensureAlways(!SuccessfulPunchEffectClass.IsNull() && !UnsuccessfulPunchEffectClass.IsNull()))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}
	
	LoadSuccessfulPunchEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SuccessfulPunchEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this]
		{
			if (WasHitBeforeEffectLoaded && DesiredToApplyGameplayEffectClass == SuccessfulPunchEffectClass)
			{
				ApplyDesiredGameplayEffect();
			}
		}));

	LoadUnsuccessfulPunchEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		UnsuccessfulPunchEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateLambda([this]
		{
			if (WasHitBeforeEffectLoaded && DesiredToApplyGameplayEffectClass == UnsuccessfulPunchEffectClass)
			{
				ApplyDesiredGameplayEffect();
			}
		}));
}

void UPunchGameplayAbility::OnStartDamageFrame(FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData)
{
	MeleeHitComponent->OnComponentBeginOverlap.AddDynamic(this, &UPunchGameplayAbility::OnHitBoxBeginOverlap);
	
	const FGameplayEventTagMulticastDelegate::FDelegate OnEndDamageFrameEventTagDelegate =
	FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnEndDamageFrame);
	
	CurrentActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(EndDamageFrameEventTag), OnEndDamageFrameEventTagDelegate);

	OnEndDamageFrameEventTagDelegateHandle = OnEndDamageFrameEventTagDelegate.GetHandle();
}

void UPunchGameplayAbility::OnEndDamageFrame(FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData)
{
	MeleeHitComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UPunchGameplayAbility::OnHitBoxBeginOverlap);
}

void UPunchGameplayAbility::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	MeleeHitComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UPunchGameplayAbility::OnHitBoxBeginOverlap);
	
	InstigatorAbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CurrentActorInfo->AvatarActor.Get());
	
	TargetAbilitySystemComponent =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor->GetOwner());
	
	if (!IsValid(InstigatorAbilitySystemComponent) || !IsValid(TargetAbilitySystemComponent))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}

	if (!TargetAbilitySystemComponent->HasMatchingGameplayTag(BlockingPunchesTag))
	{
		DesiredToApplyGameplayEffectClass = SuccessfulPunchEffectClass;
	}
	else
	{
		DesiredToApplyGameplayEffectClass = UnsuccessfulPunchEffectClass;
	}

	if (DesiredToApplyGameplayEffectClass.IsValid())
	{
		ApplyDesiredGameplayEffect();
	}
	else
	{
		WasHitBeforeEffectLoaded = true;
	}
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

void UPunchGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(
			FGameplayTagContainer(StartDamageFrameEventTag), OnStartDamageFrameEventTagDelegateHandle);

		ActorInfo->AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(
			FGameplayTagContainer(EndDamageFrameEventTag), OnEndDamageFrameEventTagDelegateHandle);
	}

	if (LoadCurrentAnimMontageHandle.IsValid())
	{
		LoadCurrentAnimMontageHandle->CancelHandle();
		LoadCurrentAnimMontageHandle.Reset();
	}
	
	if (LoadSuccessfulPunchEffectHandle.IsValid())
	{
		LoadSuccessfulPunchEffectHandle->CancelHandle();
		LoadSuccessfulPunchEffectHandle.Reset();
	}

	if (LoadUnsuccessfulPunchEffectHandle.IsValid())
	{
		LoadUnsuccessfulPunchEffectHandle->CancelHandle();
		LoadUnsuccessfulPunchEffectHandle.Reset();
	}

	InstigatorAbilitySystemComponent = nullptr;
	TargetAbilitySystemComponent = nullptr;
	DesiredToApplyGameplayEffectClass.Reset();
	WasHitBeforeEffectLoaded = false;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
