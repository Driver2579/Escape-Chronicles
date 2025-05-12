// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/PlayerOwnershipComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "Net/UnrealNetwork.h"

#if WITH_EDITOR
EDataValidationResult FPlayerOwnershipComponentGroupTableRow::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (AllowedControlledCharacterTypes.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("AllowedControlledCharacterTypes must not be empty!")));

		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

void UPlayerOwnershipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, OwningPlayerContainer);
}

TPair<FName, FPlayerOwnershipComponentGroupTableRow*> UPlayerOwnershipComponent::GetGroup() const
{
	TPair<FName, FPlayerOwnershipComponentGroupTableRow*> Result;

	Result.Key = Group.RowName;
	Result.Value = Group.GetRow<FPlayerOwnershipComponentGroupTableRow>(TEXT("GetGroup"));

	return Result;
}

void UPlayerOwnershipComponent::InitializeOwningPlayer(const FUniquePlayerID& NewOwningPlayer,
	const EControlledCharacterType ControlledCharacterType)
{
#if DO_ENSURE
	ensureAlways(!OwningPlayerContainer.OwningPlayer.IsValid());
	ensureAlways(NewOwningPlayer.IsValid());

	ensureAlways(OwningPlayerContainer.ControlledCharacterType == EControlledCharacterType::None);

	const TPair<FName, FPlayerOwnershipComponentGroupTableRow*> Group = GetGroup();

#if DO_CHECK
	check(Group.Value);
#endif

	ensureAlwaysMsgf(Group.Value->AllowedControlledCharacterTypes.Contains(ControlledCharacterType),
		TEXT("The given ControlledCharacterType is not allowed to be set as the OwningPlayer for this "
			"component instance!"));
#endif

	OwningPlayerContainer.OwningPlayer = NewOwningPlayer;

	// Broadcast the delegate
	OnOwningPlayerInitialized.Broadcast(OwningPlayerContainer.OwningPlayer,
		OwningPlayerContainer.ControlledCharacterType);

	// Clear the delegate because we don't need it anymore
	OnOwningPlayerInitialized.Clear();
}

void UPlayerOwnershipComponent::CallOrRegister_OnOwningPlayerInitialized(
	const FOnOwningPlayerInitializedDelegate::FDelegate& Callback)
{
	if (OwningPlayerContainer.OwningPlayer.IsValid())
	{
		Callback.ExecuteIfBound(OwningPlayerContainer.OwningPlayer, OwningPlayerContainer.ControlledCharacterType);
	}
	else
	{
		OnOwningPlayerInitialized.Add(Callback);
	}
}

void UPlayerOwnershipComponent::OnPostLoadObject()
{
	// Broadcast the delegate if we loaded a valid OwningPlayer and clear the delegate because we don't need it anymore
	if (OwningPlayerContainer.OwningPlayer.IsValid())
	{
		OnOwningPlayerInitialized.Broadcast(OwningPlayerContainer.OwningPlayer,
			OwningPlayerContainer.ControlledCharacterType);

		OnOwningPlayerInitialized.Clear();
	}
}