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

	// Listen for energy attribute changes to cancel the ability if the owner actor runs out of energy
	if (ensureAlways(VitalAttributeSet))
	{
		FOnGameplayAttributeValueChange& OnEnergyAttributeValueChangeDelegate =
			ActorInfo->AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				VitalAttributeSet->GetEnergyAttribute());

		OnEnergyAttributeValueChangeDelegateHandle = OnEnergyAttributeValueChangeDelegate.AddUObject(this,
			&ThisClass::OnEnergyAttributeValueChange);
	}
}

void URequireEnergyGameplayAbilityComponent::OnEnergyAttributeValueChange(
	const FOnAttributeChangeData& OnAttributeChangeData) const
{
	// Don't do anything if the energy is greater than 0
	if (OnAttributeChangeData.NewValue > 0)
	{
		return;
	}

	// === Cancel the ability if the energy has dropped to 0 ===

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

	// Unsubscribe from the energy changes to avoid its callback being called after the ability is ended
	if (ensureAlways(IsValid(VitalAttributeSet)))
	{
		FOnGameplayAttributeValueChange& OnEnergyAttributeValueChangeDelegate =
			ActorInfo->AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				VitalAttributeSet->GetEnergyAttribute());

		OnEnergyAttributeValueChangeDelegate.Remove(OnEnergyAttributeValueChangeDelegateHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}