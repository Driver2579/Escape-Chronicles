// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PunchGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

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

	if (!ensureAlways(MontagesQueue.IsValidIndex(CurrentConfigurationIndex) &&
		IsValid(CurrentActorInfo->AvatarActor.Get()) && CurrentActorInfo->AbilitySystemComponent.IsValid()))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		
		return;
	}


	// Load and play animation, setup collider, and load effects/sounds
	SetupDamagingCollider();
	LoadAndPlayAnimMontage();
	LoadGameplayEffects();
}

void UPunchGameplayAbility::SetupDamagingCollider()
{
	TWeakObjectPtr<UShapeComponent>& DamagingCollider = MontagesQueue[CurrentConfigurationIndex].DamagingCollider;

	// Search for a collider with the specified tag and assign it
	if (!DamagingCollider.IsValid())
	{
		DamagingCollider = CurrentActorInfo->AvatarActor->FindComponentByTag<UShapeComponent>(
			MontagesQueue[CurrentConfigurationIndex].DamagingColliderTag);
	}

	// If the collider was not specified and found the ability must be canceled
	if (!ensureAlways(DamagingCollider.IsValid()))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		
		return;
	}
	
	DesiredDamagingCollider = DamagingCollider;

	BeginWaitGameplayEvents();
}


void UPunchGameplayAbility::BeginWaitGameplayEvents()
{
	// === Start listening for the "StartDamageFrame" gameplay event ===
	
	const FGameplayEventTagMulticastDelegate::FDelegate OnStartDamageFrameEventTagDelegate =
		FGameplayEventTagMulticastDelegate::FDelegate::CreateLambda([this](FGameplayTag GameplayTag,
			const FGameplayEventData* GameplayEventData)
		{
			DesiredDamagingCollider->OnComponentBeginOverlap.AddDynamic(this, &UPunchGameplayAbility::OnHitBoxBeginOverlap);
		});
	
	CurrentActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(StartDamageFrameEventTag), OnStartDamageFrameEventTagDelegate);

	OnStartDamageFrameEventTagDelegateHandle = OnStartDamageFrameEventTagDelegate.GetHandle();

	// === Start listening for the "EndDamageFrame" gameplay event ===
	
	const FGameplayEventTagMulticastDelegate::FDelegate OnEndDamageFrameEventTagDelegate =
		FGameplayEventTagMulticastDelegate::FDelegate::CreateLambda([this](FGameplayTag GameplayTag,
			const FGameplayEventData* GameplayEventData)
		{
			DesiredDamagingCollider->OnComponentBeginOverlap.RemoveDynamic(this,
				&UPunchGameplayAbility::OnHitBoxBeginOverlap);
		});
	
	CurrentActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(EndDamageFrameEventTag), OnEndDamageFrameEventTagDelegate);

	OnEndDamageFrameEventTagDelegateHandle = OnEndDamageFrameEventTagDelegate.GetHandle();
}

void UPunchGameplayAbility::LoadAndPlayAnimMontage()
{
	const TSoftObjectPtr<UAnimMontage>& DesiredAnimMontage = MontagesQueue[CurrentConfigurationIndex].AnimMontage;
	
	if (!ensureAlways(!DesiredAnimMontage.IsNull()))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}
	
	LoadCurrentAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		DesiredAnimMontage.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnAnimMontageLoaded));
}

void UPunchGameplayAbility::LoadGameplayEffects()
{
	if (!ensureAlways(!SuccessfulPunchEffectClass.IsNull() && !UnsuccessfulPunchEffectClass.IsNull()))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}
	
	LoadSuccessfulPunchEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SuccessfulPunchEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&UPunchGameplayAbility::OnGameplayEffectLoaded, SuccessfulPunchEffectClass));

	LoadUnsuccessfulPunchEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		UnsuccessfulPunchEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&UPunchGameplayAbility::OnGameplayEffectLoaded, UnsuccessfulPunchEffectClass));
}

void UPunchGameplayAbility::OnAnimMontageLoaded()
{
	const TSoftObjectPtr<UAnimMontage>& DesiredAnimMontage = MontagesQueue[CurrentConfigurationIndex].AnimMontage;
	
	if (!ensureAlways(DesiredAnimMontage.IsValid()))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}

	// === Play animation and bind its end as end of ability ===
	
	CurrentActorInfo->AbilitySystemComponent->PlayMontage(this, CurrentActivationInfo, DesiredAnimMontage.Get(), 1);
	
	UAnimInstance* AnimInstance = CurrentActorInfo->GetAnimInstance();

	if (!ensureAlways(IsValid(AnimInstance)))
	{      
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		return;
	}
	
	FOnMontageBlendingOutStarted OnAnimMontageBlendingOutDelegate =
		FOnMontageBlendingOutStarted::CreateUObject(this, &UPunchGameplayAbility::OnAnimMontageBlendingOut);

	// We use “BlendingOut” rather than “End” because it gives the use of the MontagesQueue as a queue of combo punches
	AnimInstance->Montage_SetBlendingOutDelegate(OnAnimMontageBlendingOutDelegate, DesiredAnimMontage.Get());

	// Cycle to the next montage in the queue
	CurrentConfigurationIndex = (CurrentConfigurationIndex + 1) % MontagesQueue.Num();
}

void UPunchGameplayAbility::OnAnimMontageBlendingOut(UAnimMontage* AnimMontage, bool bInterrupted)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPunchGameplayAbility::OnGameplayEffectLoaded(const TSoftClassPtr<UGameplayEffect> LoadedEffect)
{
	// Apply effect if punch occurred but resource was not loaded by that time
	if (IsPunchHappened && ensureAlways(DesiredToApplyGameplayEffectClass.IsValid()) &&
		DesiredToApplyGameplayEffectClass == LoadedEffect)
	{
		ApplyDesiredGameplayEffect();
	}
}

void UPunchGameplayAbility::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Ignore the overlaps on yourself
	if (CurrentActorInfo->AvatarActor == OtherActor)
	{
		return;
	}

	UAbilitySystemComponent* InstigatorAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	if (!ensureAlways(IsValid(InstigatorAbilitySystemComponent)))
	{
		return;
	}

	TargetAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);

	DesiredDamagingCollider->OnComponentBeginOverlap.RemoveDynamic(this, &UPunchGameplayAbility::OnHitBoxBeginOverlap);
	IsPunchHappened = true;

	// We can hit an object without ASC, but it will only cause visual effects
	if (!TargetAbilitySystemComponent.IsValid())
	{
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(SuccessfulPunchGameplayCueTag.GameplayCueTag);
		
		return;
	}

	// Determine if target has a blocking tag
	if (!TargetAbilitySystemComponent->HasMatchingGameplayTag(BlockingPunchesTag))
	{
		DesiredToApplyGameplayEffectClass = SuccessfulPunchEffectClass;
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(SuccessfulPunchGameplayCueTag.GameplayCueTag);
	}
	else
	{
		DesiredToApplyGameplayEffectClass = UnsuccessfulPunchEffectClass;
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(SuccessfulPunchGameplayCueTag.GameplayCueTag);
	}

	// == Whenever possible, we use desired asset ===
	
	if (DesiredToApplyGameplayEffectClass.IsValid())
	{
		ApplyDesiredGameplayEffect();
	}
}

void UPunchGameplayAbility::ApplyDesiredGameplayEffect() const
{
	if (!CurrentActorInfo->AbilitySystemComponent.IsValid() || !TargetAbilitySystemComponent.IsValid() ||
		!DesiredToApplyGameplayEffectClass.IsValid())
	{
		return;
	}
	
	const FGameplayEffectSpecHandle EffectSpecHandle = CurrentActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
		DesiredToApplyGameplayEffectClass.Get(), GetAbilityLevel(), FGameplayEffectContextHandle());
	
	CurrentActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),
		TargetAbilitySystemComponent.Get());
}

void UPunchGameplayAbility::UnloadByHandle(TSharedPtr<FStreamableHandle>& Handle)
{
	if (Handle.IsValid())
	{
		Handle->CancelHandle();
		Handle.Reset();
	}
}

void UPunchGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove registered gameplay event delegates
	if (ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(
			FGameplayTagContainer(StartDamageFrameEventTag), OnStartDamageFrameEventTagDelegateHandle);

		ActorInfo->AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(
			FGameplayTagContainer(EndDamageFrameEventTag), OnEndDamageFrameEventTagDelegateHandle);
	}

	// Unload all async handles
	UnloadByHandle(LoadCurrentAnimMontageHandle);
	UnloadByHandle(LoadSuccessfulPunchEffectHandle);
	UnloadByHandle(LoadUnsuccessfulPunchEffectHandle);

	// Clear references
	TargetAbilitySystemComponent = nullptr;
	IsPunchHappened = false;
	DesiredToApplyGameplayEffectClass.Reset();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
