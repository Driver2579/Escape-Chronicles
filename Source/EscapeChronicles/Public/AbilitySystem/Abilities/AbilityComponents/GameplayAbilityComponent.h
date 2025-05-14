// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppPassValueParameterByConstReference
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"
#include "GameplayAbilityComponent.generated.h"

/**
 * Base class for modular components that can be added to gameplay abilities in this project. It copies almost the whole
 * interface of UGameplayAbility.
 */
UCLASS(Abstract, Const, DefaultToInstanced, EditInlineNew, CollapseCategories, Within=EscapeChroniclesGameplayAbility)
class ESCAPECHRONICLES_API UGameplayAbilityComponent : public UObject
{
	GENERATED_BODY()

public:
	// Returns the GameplayEffect that owns this Component (the Outer)
	UEscapeChroniclesGameplayAbility* GetOwner() const { return GetTypedOuter<UEscapeChroniclesGameplayAbility>(); }

	// === UGameplayAbility interface ===

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const
	{
		return true;
	}

	virtual bool ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayEventData* Payload) const
	{
		return true;
	}

	virtual bool ShouldActivateAbility(ENetRole Role) const
	{
		return true;
	}

	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const
	{
		return true;
	}

	virtual bool IsBlockingOtherAbilities() const { return false; }

	virtual bool CanBeCanceled() const { return true; }

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) {}

	virtual bool CommitCheck(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr)
	{
		return true;
	}

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const
	{
		return true;
	}

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const
	{
		return true;
	}

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const {}

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const {}

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) {}

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) {}

	virtual void OnWaitingForConfirmInputBegin() {}
	virtual void OnWaitingForConfirmInputEnd() {}

	virtual void SetCurrentMontage(UAnimMontage* InCurrentMontage) {}

	virtual void SetRemoteInstanceHasEnded() {}

	virtual void NotifyAvatarDestroyed() {}

	virtual void NotifyAbilityTaskWaitingOnPlayerData(UAbilityTask* AbilityTask) {}
	virtual void NotifyAbilityTaskWaitingOnAvatar(UAbilityTask* AbilityTask) {}

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) {}
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) {}

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) {}

	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) {}
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) {}
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) {}

	// Called only if CommitCheck returns true
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {}

	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
		const FGameplayEventData* TriggerEventData = nullptr) {}

	virtual void ConfirmActivateSucceed() {}

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) {}

	virtual void SetCurrentActorInfo(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo) const {}

	virtual void SetCurrentActivationInfo(const FGameplayAbilityActivationInfo ActivationInfo) {}

	// === End of UGameplayAbility interface ===

protected:
	// === Start of UGameplayAbility interface ===

	void SetAssetTags(const FGameplayTagContainer& InAbilityTags)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->SetAssetTags(InAbilityTags);
	}

	virtual void SendGameplayEvent(FGameplayTag EventTag, FGameplayEventData Payload)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->SendGameplayEvent(EventTag, Payload);
	}

	void CallActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate = nullptr,
		const FGameplayEventData* TriggerEventData = nullptr)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->CallActivateAbility(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate,
			TriggerEventData);
	}

	bool IsEndAbilityValid(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		return GetOwner()->IsEndAbilityValid(Handle, ActorInfo);
	}

	FActiveGameplayEffectHandle ApplyGameplayEffectToOwner(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const UGameplayEffect* GameplayEffect, float GameplayEffectLevel, int32 Stacks = 1) const
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		return GetOwner()->ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, GameplayEffect,
			GameplayEffectLevel, Stacks);
	}

	FActiveGameplayEffectHandle ApplyGameplayEffectSpecToOwner(const FGameplayAbilitySpecHandle AbilityHandle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEffectSpecHandle SpecHandle) const
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		return GetOwner()->ApplyGameplayEffectSpecToOwner(AbilityHandle, ActorInfo, ActivationInfo, SpecHandle);
	}

	TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectToTarget(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayAbilityTargetDataHandle& Target, TSubclassOf<UGameplayEffect> GameplayEffectClass,
		float GameplayEffectLevel, int32 Stacks = 1) const
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		return GetOwner()->ApplyGameplayEffectToTarget(Handle, ActorInfo, ActivationInfo, Target, GameplayEffectClass,
			GameplayEffectLevel, Stacks);
	}

	TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectSpecToTarget(const FGameplayAbilitySpecHandle AbilityHandle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEffectSpecHandle SpecHandle, const FGameplayAbilityTargetDataHandle& TargetData) const
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		return GetOwner()->ApplyGameplayEffectSpecToTarget(AbilityHandle, ActorInfo, ActivationInfo, SpecHandle,
			TargetData);
	}

	void ConfirmTaskByInstanceName(const FName& InstanceName, bool bEndTask)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->ConfirmTaskByInstanceName(InstanceName, bEndTask);
	}

	void CancelTaskByInstanceName(const FName& InstanceName)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->CancelTaskByInstanceName(InstanceName);
	}

	void EndAbilityState(const FName& OptionalStateNameToEnd)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->EndAbilityState(OptionalStateNameToEnd);
	}

	void MontageJumpToSection(const FName& SectionName)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->MontageJumpToSection(SectionName);
	}

	void MontageSetNextSectionName(const FName& FromSectionName, const FName& ToSectionName)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->MontageSetNextSectionName(FromSectionName, ToSectionName);
	}

	void MontageStop(float OverrideBlendOutTime = -1.0f)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->MontageStop(OverrideBlendOutTime);
	}

	FGameplayAbilityTargetingLocationInfo MakeTargetLocationInfoFromOwnerActor()
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		return GetOwner()->MakeTargetLocationInfoFromOwnerActor();
	}

	FGameplayAbilityTargetingLocationInfo MakeTargetLocationInfoFromOwnerSkeletalMeshComponent(const FName& SocketName)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		return GetOwner()->MakeTargetLocationInfoFromOwnerSkeletalMeshComponent(SocketName);
	}

	void SetCurrentInfo(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->SetCurrentInfo(Handle, ActorInfo, ActivationInfo);
	}

	void IncrementListLock() const
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->IncrementListLock();
	}

	void DecrementListLock() const
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->DecrementListLock();
	}

	// === End of UGameplayAbility interface ===

	// Calls EndAbility on the owner
	void CallEndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const bool bReplicateEndAbility, const bool bWasCancelled)
	{
#if DO_CHECK
		check(IsValid(GetOwner()));
#endif

		GetOwner()->EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
};