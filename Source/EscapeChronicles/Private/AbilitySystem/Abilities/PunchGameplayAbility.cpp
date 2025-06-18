// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PunchGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Structs/CombatEvents.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Objects/InventoryItemFragments/WeaponInventoryItemFragment.h"

void UPunchGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

#if DO_CHECK
	check(CurrentActorInfo);
	check(CurrentActorInfo->AvatarActor.IsValid());
	check(CurrentActorInfo->AbilitySystemComponent.IsValid());
#endif

	SetupUsingWeapon();

	const bool bSuccess = CommitAbility(Handle, ActorInfo, ActivationInfo) && LoadAndPlayAnimMontage() &&
		LoadGameplayEffects() && SetupDamageCollision();

	if (!bSuccess)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UPunchGameplayAbility::SetupUsingWeapon()
{
	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(CurrentActorInfo->AvatarActor);

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	const UInventoryManagerSelectorFragment* InventoryManagerSelectorFragment =
		Inventory->GetFragmentByClass<UInventoryManagerSelectorFragment>();

	if (!ensureAlways(IsValid(InventoryManagerSelectorFragment)))
	{
		return;
	}

	const int32 Index = InventoryManagerSelectorFragment->GetCurrentSlotIndex();
	const FGameplayTag& SlotTypeTag = InventoryManagerSelectorFragment->GetSelectableSlotsTypeTag();

	const UInventoryItemInstance* ItemInstance = Inventory->GetItemInstance(Index, SlotTypeTag);

	if (!IsValid(ItemInstance))
	{
		return;
	}

	UWeaponInventoryItemFragment* WeaponInventoryItemFragment =
		ItemInstance->GetFragmentByClass<UWeaponInventoryItemFragment>();

	if (!IsValid(WeaponInventoryItemFragment))
	{
		return;
	}

	UsingWeaponFragment = WeaponInventoryItemFragment;

	UsingWeapon = WeaponInventoryItemFragment->GetActor(ItemInstance);
}

bool UPunchGameplayAbility::SetupDamageCollision()
{
	if (UsingWeaponFragment.IsValid() && ensureAlways(UsingWeapon.IsValid()))
	{
		DesiredDamageCollision =
			UsingWeapon->FindComponentByTag<UPrimitiveComponent>(UsingWeaponFragment->GetDamageCollisionTag());

		RegisterPunchGameplayEvents();

		return true;
	}

	TWeakObjectPtr<UPrimitiveComponent>& DamageCollision = MontagesQueue[CurrentConfigurationIndex].DamageCollision;

	// Search for a collision with the specified tag and assign it
	if (!DamageCollision.IsValid())
	{
		// TODO: Rework to support several collisions instead of getting just one
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
#if DO_CHECK
	check(MontagesQueue.IsValidIndex(CurrentConfigurationIndex));
#endif

	if (UsingWeaponFragment.IsValid())
	{
		LoadCurrentAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			UsingWeaponFragment->GetAnimMontage().ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
				&ThisClass::OnAnimMontageLoaded));
		return true;
	}

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
		SuccessfulPunchGameplayEffectClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameplayEffectLoaded,
			SuccessfulPunchGameplayEffectClass));

	LoadUnsuccessfulPunchEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		UnsuccessfulPunchGameplayEffectClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameplayEffectLoaded,
			UnsuccessfulPunchGameplayEffectClass));

	return true;
}

void UPunchGameplayAbility::OnAnimMontageLoaded()
{
#if DO_CHECK
	check(MontagesQueue.IsValidIndex(CurrentConfigurationIndex));
#endif

	const TSoftObjectPtr<UAnimMontage>& DesiredAnimMontage = UsingWeaponFragment.IsValid() ?
		UsingWeaponFragment->GetAnimMontage() :
		MontagesQueue[CurrentConfigurationIndex].AnimMontage;
	
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

	const bool bDesiredEffectLoaded = ensureAlways(DesiredGameplayEffectClassToApply.IsValid()) &&
		DesiredGameplayEffectClassToApply == LoadedEffect;
	
	// Apply effect if punch occurred but resource was not loaded by that time
	if (bDesiredEffectLoaded)
	{
		ApplyDesiredGameplayEffectToTargetChecked();
	}
}

void UPunchGameplayAbility::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
#if DO_CHECK
	check(CurrentActorInfo);
	check(CurrentActorInfo->AvatarActor.IsValid());
	check(CurrentActorInfo->AbilitySystemComponent.IsValid());
#endif

	// Ignore the overlaps on yourself
	if (CurrentActorInfo->AvatarActor == OtherActor)
	{
		return;
	}

	UAbilitySystemComponent* InstigatorAbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get();

	TargetAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);

	DesiredDamageCollision->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnHitBoxBeginOverlap);
	bPunchHappened = true;

	/**
	 * The hit may not only be on the character (for example, a wall), so TargetAbilitySystemComponent may be nullptr.
	 * In that case we only execute visual effects
	 */
	if (!TargetAbilitySystemComponent.IsValid())
	{
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(SuccessfulPunchGameplayCueTag.GameplayCueTag);

		return;
	}

	// Determine if target has a blocking tag
	if (!TargetAbilitySystemComponent->HasAnyMatchingGameplayTags(BlockingPunchesTags))
	{
		DesiredGameplayEffectClassToApply = SuccessfulPunchGameplayEffectClass;
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(SuccessfulPunchGameplayCueTag.GameplayCueTag);
	}
	else
	{
		DesiredGameplayEffectClassToApply = UnsuccessfulPunchGameplayEffectClass;
		InstigatorAbilitySystemComponent->ExecuteGameplayCue(UnsuccessfulPunchGameplayCueTag.GameplayCueTag);
	}

	// === Whenever possible, we use a desired asset ===
	
	if (DesiredGameplayEffectClassToApply.IsValid())
	{
		ApplyDesiredGameplayEffectToTargetChecked();
	}

	if (UsingWeaponFragment.IsValid())
	{
		UsingWeaponFragment->EffectHit();
	}
}

void UPunchGameplayAbility::ApplyDesiredGameplayEffectToTargetChecked() const
{
#if DO_CHECK
	check(CurrentActorInfo);
	check(CurrentActorInfo->AbilitySystemComponent.IsValid());
	check(TargetAbilitySystemComponent.IsValid());
	check(DesiredGameplayEffectClassToApply.IsValid());
#endif

	const FActiveGameplayEffectHandle AppliedEffectHandle =
		CurrentActorInfo->AbilitySystemComponent->ApplyGameplayEffectToTarget(
			DesiredGameplayEffectClassToApply->GetDefaultObject<UGameplayEffect>(),
			TargetAbilitySystemComponent.Get(), GetAbilityLevel());

	FCombatEvents::OnPunchHit.Broadcast(CurrentActorInfo->AbilitySystemComponent.Get(),
		TargetAbilitySystemComponent.Get(), AppliedEffectHandle);
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
	DesiredGameplayEffectClassToApply.Reset();

	UsingWeapon = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
