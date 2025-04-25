// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/AbilityComponents/RequireEnergyGameplayAbilityComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"

bool URequireEnergyGameplayAbilityComponent::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	const UVitalAttributeSet* VitalAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UVitalAttributeSet>();

	if (!ensureAlways(VitalAttributeSet))
	{
		return true;
	}

	return VitalAttributeSet->GetEnergy() > 0;
}

void URequireEnergyGameplayAbilityComponent::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const UVitalAttributeSet* VitalAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UVitalAttributeSet>();

	if (ensureAlways(VitalAttributeSet))
	{
		// Listen to the OnOutOfEnergy event to cancel the ability if the owner actor runs out of energy
		OnOutOfEnergyDelegateHandle = VitalAttributeSet->OnOutOfEnergy.AddUObject(this, &ThisClass::OnOutOfEnergy);
	}
}

void URequireEnergyGameplayAbilityComponent::OnOutOfEnergy(AActor* EffectInstigator, AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue) const
{
	UEscapeChroniclesGameplayAbility* OwningAbility = GetOwner();

#if DO_CHECK
	check(IsValid(OwningAbility));
#endif

	OwningAbility->CancelAbility(OwningAbility->GetCurrentAbilitySpecHandle(), OwningAbility->GetCurrentActorInfo(),
		OwningAbility->GetCurrentActivationInfoRef(), true);
}

void URequireEnergyGameplayAbilityComponent::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	const UVitalAttributeSet* VitalAttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UVitalAttributeSet>();

	if (ensureAlways(IsValid(VitalAttributeSet)))
	{
		// Unsubscribe from the OnOutOfEnergy event to avoid its callback being called after the ability is ended
		VitalAttributeSet->OnOutOfEnergy.Remove(OnOutOfEnergyDelegateHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}