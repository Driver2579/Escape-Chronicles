// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/EscapeChroniclesGameplayAbility.h"

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
