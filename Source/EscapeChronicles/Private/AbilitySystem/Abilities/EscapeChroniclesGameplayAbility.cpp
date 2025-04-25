// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"

#include "AbilitySystem/Abilities/AbilityComponents/GameplayAbilityComponent.h"
#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"

UEscapeChroniclesGameplayAbility::UEscapeChroniclesGameplayAbility()
{
	/**
	 * Don't replicate abilities by default. This should be changed manually for each ability that needs to be
	 * replicated.
	 */
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;

	// Only InstancedPerActor abilities support replication at the moment
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	/**
	 * Most of the time we want to be able to activate any ability on both the client and the server and then replicate
	 * this activation. It's best to use LocalPredicted to predict abilities activation on client to save network
	 * performance.
	 */
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}

bool UEscapeChroniclesGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if all components allow ability activation
	for (const UGameplayAbilityComponent* Component : Components)
	{
		if (!Component->CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		{
			return false;
		}
	}

	return true;
}

bool UEscapeChroniclesGameplayAbility::ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayEventData* Payload) const
{
	if (!Super::ShouldAbilityRespondToEvent(ActorInfo, Payload))
	{
		return false;
	}

	// Check if all components allow the ability to respond to the event
	for (const UGameplayAbilityComponent* Component : Components)
	{
		if (!Component->ShouldAbilityRespondToEvent(ActorInfo, Payload))
		{
			return false;
		}
	}

	return true;
}

bool UEscapeChroniclesGameplayAbility::ShouldActivateAbility(ENetRole Role) const
{
	if (!Super::ShouldActivateAbility(Role))
	{
		return false;
	}

	// Check if all components allow ability activation
	for (const UGameplayAbilityComponent* Component : Components)
	{
		if (!Component->ShouldActivateAbility(Role))
		{
			return false;
		}
	}

	return true;
}

bool UEscapeChroniclesGameplayAbility::DoesAbilitySatisfyTagRequirements(
	const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	const bool bParentResult = Super::DoesAbilitySatisfyTagRequirements(AbilitySystemComponent, SourceTags, TargetTags,
		OptionalRelevantTags);

	if (!bParentResult)
	{
		return false;
	}

	// Check if all components satisfy tag requirements
	for (const UGameplayAbilityComponent* Component : Components)
	{
		const bool bComponentResult = Component->DoesAbilitySatisfyTagRequirements(AbilitySystemComponent, SourceTags,
			TargetTags, OptionalRelevantTags);

		if (!bComponentResult)
		{
			return false;
		}
	}

	return true;
}

bool UEscapeChroniclesGameplayAbility::IsBlockingOtherAbilities() const
{
	if (Super::IsBlockingOtherAbilities())
	{
		return true;
	}

	// Check if any component is blocking other abilities
	for (const UGameplayAbilityComponent* Component : Components)
	{
		if (Component->IsBlockingOtherAbilities())
		{
			return true;
		}
	}

	return false;
}

bool UEscapeChroniclesGameplayAbility::CanBeCanceled() const
{
	if (!Super::CanBeCanceled())
	{
		return false;
	}

	// Check if any component blocks ability cancellation
	for (const UGameplayAbilityComponent* Component : Components)
	{
		if (!Component->CanBeCanceled())
		{
			return false;
		}
	}

	return true;
}

void UEscapeChroniclesGameplayAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
	}

	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

bool UEscapeChroniclesGameplayAbility::CommitCheck(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	if (!Super::CommitCheck(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
	{
		return false;
	}

	// Check if all components satisfy the commit check
	for (UGameplayAbilityComponent* Component : Components)
	{
		if (!Component->CommitCheck(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags))
		{
			return false;
		}
	}

	return true;
}

bool UEscapeChroniclesGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	// Check if all components satisfy the cooldown check
	for (const UGameplayAbilityComponent* Component : Components)
	{
		if (!Component->CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
		{
			return false;
		}
	}

	return true;
}

bool UEscapeChroniclesGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	// Check if all components satisfy the cost check
	for (const UGameplayAbilityComponent* Component : Components)
	{
		if (!Component->CheckCost(Handle, ActorInfo, OptionalRelevantTags))
		{
			return false;
		}
	}

	return true;
}

void UEscapeChroniclesGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);

	for (const UGameplayAbilityComponent* Component : Components)
	{
		Component->ApplyCooldown(Handle, ActorInfo, ActivationInfo);
	}
}

void UEscapeChroniclesGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	for (const UGameplayAbilityComponent* Component : Components)
	{
		Component->ApplyCost(Handle, ActorInfo, ActivationInfo);
	}
}

void UEscapeChroniclesGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->InputPressed(Handle, ActorInfo, ActivationInfo);
	}
}

void UEscapeChroniclesGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->InputReleased(Handle, ActorInfo, ActivationInfo);
	}
}

void UEscapeChroniclesGameplayAbility::OnWaitingForConfirmInputBegin()
{
	Super::OnWaitingForConfirmInputBegin();

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->OnWaitingForConfirmInputBegin();
	}
}

void UEscapeChroniclesGameplayAbility::OnWaitingForConfirmInputEnd()
{
	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->OnWaitingForConfirmInputEnd();
	}

	Super::OnWaitingForConfirmInputEnd();
}

void UEscapeChroniclesGameplayAbility::SetCurrentMontage(UAnimMontage* InCurrentMontage)
{
	Super::SetCurrentMontage(InCurrentMontage);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->SetCurrentMontage(InCurrentMontage);
	}
}

void UEscapeChroniclesGameplayAbility::SetRemoteInstanceHasEnded()
{
	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->SetRemoteInstanceHasEnded();
	}

	Super::SetRemoteInstanceHasEnded();
}

void UEscapeChroniclesGameplayAbility::NotifyAvatarDestroyed()
{
	Super::NotifyAvatarDestroyed();

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->NotifyAvatarDestroyed();
	}
}

void UEscapeChroniclesGameplayAbility::NotifyAbilityTaskWaitingOnPlayerData(UAbilityTask* AbilityTask)
{
	Super::NotifyAbilityTaskWaitingOnPlayerData(AbilityTask);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->NotifyAbilityTaskWaitingOnPlayerData(AbilityTask);
	}
}

void UEscapeChroniclesGameplayAbility::NotifyAbilityTaskWaitingOnAvatar(UAbilityTask* AbilityTask)
{
	Super::NotifyAbilityTaskWaitingOnAvatar(AbilityTask);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->NotifyAbilityTaskWaitingOnAvatar(AbilityTask);
	}
}

void UEscapeChroniclesGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->OnGiveAbility(ActorInfo, Spec);
	}
}

void UEscapeChroniclesGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->OnRemoveAbility(ActorInfo, Spec);
	}

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UEscapeChroniclesGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	for	(UGameplayAbilityComponent* Component : Components)
	{
		Component->OnAvatarSet(ActorInfo, Spec);
	}
}

void UEscapeChroniclesGameplayAbility::OnGameplayTaskInitialized(UGameplayTask& Task)
{
	Super::OnGameplayTaskInitialized(Task);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->OnGameplayTaskInitialized(Task);
	}
}

void UEscapeChroniclesGameplayAbility::OnGameplayTaskActivated(UGameplayTask& Task)
{
	Super::OnGameplayTaskActivated(Task);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->OnGameplayTaskActivated(Task);
	}
}

void UEscapeChroniclesGameplayAbility::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->OnGameplayTaskDeactivated(Task);
	}

	Super::OnGameplayTaskDeactivated(Task);
}

void UEscapeChroniclesGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Call ActivateAbility on components only if CommitCheck returns true
	if (CommitCheck(Handle, ActorInfo, ActivationInfo))
	{
		for (UGameplayAbilityComponent* Component : Components)
		{
			Component->ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
		}
	}
}

void UEscapeChroniclesGameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
	}
}

void UEscapeChroniclesGameplayAbility::ConfirmActivateSucceed()
{
	Super::ConfirmActivateSucceed();

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->ConfirmActivateSucceed();
	}
}

void UEscapeChroniclesGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UEscapeChroniclesGameplayAbility::SetCurrentActorInfo(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	Super::SetCurrentActorInfo(Handle, ActorInfo);

	for (const UGameplayAbilityComponent* Component : Components)
	{
		Component->SetCurrentActorInfo(Handle, ActorInfo);
	}
}

void UEscapeChroniclesGameplayAbility::SetCurrentActivationInfo(const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::SetCurrentActivationInfo(ActivationInfo);

	for (UGameplayAbilityComponent* Component : Components)
	{
		Component->SetCurrentActivationInfo(ActivationInfo);
	}
}