// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PunchGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
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

	if (!ensureAlways(MontagesQueue.IsValidIndex(CurrentConfigurationIndex) &&
		IsValid(CurrentActorInfo->AvatarActor.Get()) && CurrentActorInfo->AbilitySystemComponent.IsValid()))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		
		return;
	}

	if (!SetupDamageCollision() || !LoadAndPlayAnimMontage() || !LoadGameplayEffects())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

bool UPunchGameplayAbility::SetupDamageCollision()
{
	TWeakObjectPtr<UPrimitiveComponent>& DamageCollision = MontagesQueue[CurrentConfigurationIndex].DamageCollision;

	// Search for a collision with the specified tag and assign it
	if (!DamageCollision.IsValid())
	{
		DamageCollision = CurrentActorInfo->AvatarActor->FindComponentByTag<UPrimitiveComponent>(
			MontagesQueue[CurrentConfigurationIndex].DamageCollisionTag);
	}

	// If the collision was not specified and found the ability must be canceled
	if (!ensureAlways(DamageCollision.IsValid()))
	{
		return false;
	}
	
	DesiredDamageCollision = DamageCollision;

	RegisterPunchGameplayEvents();

	return true;
}


void UPunchGameplayAbility::RegisterPunchGameplayEvents()
{
	// === Start listening for the "StartDamageFrame" gameplay event ===
	
	const FGameplayEventTagMulticastDelegate::FDelegate OnStartDamageFrameEventTagDelegate =
		FGameplayEventTagMulticastDelegate::FDelegate::CreateWeakLambda(this,
			[this](FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData)
		{
			DesiredDamageCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnHitBoxBeginOverlap);
		});
	
	CurrentActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(StartDamageFrameEventTag), OnStartDamageFrameEventTagDelegate);

	OnStartDamageFrameEventTagDelegateHandle = OnStartDamageFrameEventTagDelegate.GetHandle();

	// === Start listening for the "EndDamageFrame" gameplay event ===
	
	const FGameplayEventTagMulticastDelegate::FDelegate OnEndDamageFrameEventTagDelegate =
		FGameplayEventTagMulticastDelegate::FDelegate::CreateWeakLambda(this,
			[this](FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData)
		{
			DesiredDamageCollision->OnComponentBeginOverlap.RemoveDynamic(this,
				&ThisClass::OnHitBoxBeginOverlap);
		});
	
	CurrentActorInfo->AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		FGameplayTagContainer(EndDamageFrameEventTag), OnEndDamageFrameEventTagDelegate);

	OnEndDamageFrameEventTagDelegateHandle = OnEndDamageFrameEventTagDelegate.GetHandle();
}

bool UPunchGameplayAbility::LoadAndPlayAnimMontage()
{
	check(CurrentConfigurationIndex >= 0 && CurrentConfigurationIndex < MontagesQueue.Num());
	
	const TSoftObjectPtr<UAnimMontage>& DesiredAnimMontage = MontagesQueue[CurrentConfigurationIndex].AnimMontage;
	
	if (!ensureAlways(!DesiredAnimMontage.IsNull()))
	{
		return false;
	}
	
	LoadCurrentAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		DesiredAnimMontage.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnAnimMontageLoaded));

	return true;
}

bool UPunchGameplayAbility::LoadGameplayEffects()
{
	if (!ensureAlways(!SuccessfulPunchGameplayEffectClass.IsNull()) || !ensureAlways(!UnsuccessfulPunchGameplayEffectClass.IsNull()))
	{
		return false;
	}
	
	LoadSuccessfulPunchEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SuccessfulPunchGameplayEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnGameplayEffectLoaded, SuccessfulPunchGameplayEffectClass));

	LoadUnsuccessfulPunchEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		UnsuccessfulPunchGameplayEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnGameplayEffectLoaded, UnsuccessfulPunchGameplayEffectClass));

	return true;
}

void UPunchGameplayAbility::OnAnimMontageLoaded()
{
	check(CurrentConfigurationIndex >= 0 && CurrentConfigurationIndex < MontagesQueue.Num());
	
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
		FOnMontageBlendingOutStarted::CreateUObject(this, &ThisClass::OnAnimMontageBlendingOut);

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
	if (!bPunchHappened)
	{
		return;
	}
	
	const bool IsLoadedDesiredEffect = ensureAlways(DesiredToApplyGameplayEffectClass.IsValid()) &&
		DesiredToApplyGameplayEffectClass == LoadedEffect;
	
	// Apply effect if punch occurred but resource was not loaded by that time
	if (IsLoadedDesiredEffect)
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

	check(CurrentActorInfo->AbilitySystemComponent.Get());
	
	UAbilitySystemComponent* InstigatorAbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get();

	TargetAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);

	DesiredDamageCollision->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnHitBoxBeginOverlap);
	bPunchHappened = true;

	// We can hit an object without ASC, so it will only cause visual effects
	if (!TargetAbilitySystemComponent.IsValid())
	{
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(SuccessfulPunchGameplayCueTag.GameplayCueTag);
		
		return;
	}

	// Determine if target has a blocking tag
	if (!TargetAbilitySystemComponent->HasMatchingGameplayTag(BlockingPunchesTag))
	{
		DesiredToApplyGameplayEffectClass = SuccessfulPunchGameplayEffectClass;
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(SuccessfulPunchGameplayCueTag.GameplayCueTag);
	}
	else
	{
		DesiredToApplyGameplayEffectClass = UnsuccessfulPunchGameplayEffectClass;
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(UnsuccessfulPunchGameplayCueTag.GameplayCueTag);
	}

	// == Whenever possible, we use a desired asset ===
	
	if (DesiredToApplyGameplayEffectClass.IsValid())
	{
		ApplyDesiredGameplayEffect();
	}
}

void UPunchGameplayAbility::ApplyDesiredGameplayEffect() const
{
	check(CurrentActorInfo->AbilitySystemComponent.IsValid());
	
	if (!TargetAbilitySystemComponent.IsValid() || !ensureAlways(DesiredToApplyGameplayEffectClass.IsValid()))
	{
		return;
	}
	
	const FGameplayEffectSpecHandle EffectSpecHandle = CurrentActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
		DesiredToApplyGameplayEffectClass.Get(), GetAbilityLevel(), FGameplayEffectContextHandle());
	
	CurrentActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(),
		TargetAbilitySystemComponent.Get());
}

void UPunchGameplayAbility::UnloadSoftObject(TSharedPtr<FStreamableHandle>& Handle)
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

	// Unload all soft objects
	UnloadSoftObject(LoadCurrentAnimMontageHandle);
	UnloadSoftObject(LoadSuccessfulPunchEffectHandle);
	UnloadSoftObject(LoadUnsuccessfulPunchEffectHandle);

	// Clear references
	TargetAbilitySystemComponent = nullptr;
	bPunchHappened = false;
	DesiredToApplyGameplayEffectClass.Reset();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
