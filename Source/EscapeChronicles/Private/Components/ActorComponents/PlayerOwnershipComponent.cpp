// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/PlayerOwnershipComponent.h"

#include "Net/UnrealNetwork.h"

void UPlayerOwnershipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, OwningPlayer);
}

void UPlayerOwnershipComponent::InitializeOwningPlayer(const FUniquePlayerID& NewOwningPlayer)
{
#if DO_ENSURE
	ensureAlways(!OwningPlayer.IsValid());
	ensureAlways(NewOwningPlayer.IsValid());
#endif

	OwningPlayer = NewOwningPlayer;

	// Broadcast the delegate and clear it because we don't need it anymore
	OnOwningPlayerInitialized.Broadcast(OwningPlayer);
	OnOwningPlayerInitialized.Clear();
}

void UPlayerOwnershipComponent::CallOrRegister_OnOwningPlayerInitialized(
	const FOnOwningPlayerInitializedDelegate::FDelegate& Callback)
{
	if (OwningPlayer.IsValid())
	{
		Callback.ExecuteIfBound(OwningPlayer);
	}
	else
	{
		OnOwningPlayerInitialized.Add(Callback);
	}
}