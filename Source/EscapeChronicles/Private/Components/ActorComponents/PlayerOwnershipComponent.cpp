// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/PlayerOwnershipComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

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

FPlayerOwnershipComponentGroup UPlayerOwnershipComponent::GetGroup() const
{
	FPlayerOwnershipComponentGroup Result;

	Result.GroupName = GroupData.RowName;
	Result.GroupSettings = GroupData.GetRow<FPlayerOwnershipComponentGroupTableRow>(TEXT("GetGroup"));

	return Result;
}

void UPlayerOwnershipComponent::RegisterPlayer(const AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

#if DO_ENSURE
	ensureAlways(!PlayerState->IsSpectator());
#endif

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

	// The group the given player is assigned to
	FName GroupNameForPlayer;

	// The map of the groups and the components that belong to this group
	TMultiMap<FPlayerOwnershipComponentGroup, UPlayerOwnershipComponent*> GroupedComponents;

	// Collect all PlayerOwnershipComponents and try to find if the given player is already assigned to one of them
	for (FActorIterator It(PlayerState->GetWorld()); It; ++It)
	{
		UPlayerOwnershipComponent* PlayerOwnershipComponent = It->FindComponentByClass<UPlayerOwnershipComponent>();

		if (!IsValid(PlayerOwnershipComponent))
		{
			continue;
		}

		const FUniquePlayerID* OwningPlayer = PlayerOwnershipComponent->GetOwningPlayer();
		const FPlayerOwnershipComponentGroup Group = PlayerOwnershipComponent->GetGroup();

		// Check if the component already has an OwningPlayer and if it's the given player
		if (OwningPlayer && *OwningPlayer == UniquePlayerID)
		{
#if DO_ENSURE
			// Check if multiple components with different groups don't have the same OwningPlayer
			ensureAlwaysMsgf(
				GroupNameForPlayer.IsNone() || Group.GroupName == GroupNameForPlayer,
				TEXT("Different components with different groups have the same OwningPlayer, which is "
					"wrong!"));

			// Check if OwningPlayer of this component is allowed to be set on this component
			ensureAlwaysMsgf(CanAssignOwningPlayerToGroup(PlayerState, Group),
				TEXT("The component has an OwningPlayer that isn't allowed to be set on this component!"));
#endif

			// Set the group name for the given player since we found a component that owns the given player
			GroupNameForPlayer = Group.GroupName;
		}

		// Add the component to the map of grouped components
		GroupedComponents.Add(Group, PlayerOwnershipComponent);
	}

	// Get all created groups
	TArray<FPlayerOwnershipComponentGroup> Groups;
	GroupedComponents.GetKeys(Groups);

	const bool bPlayerAlreadyHasGroup = !GroupNameForPlayer.IsNone();

	// This property is needed only for the "ensure" macro, so we wrap it with DO_ENSURE
#if DO_ENSURE
	bool bFoundGroup = false;
#endif

	for (const FPlayerOwnershipComponentGroup& Group : Groups)
	{
#if DO_CHECK
		check(Group.GroupSettings);
#endif

		/**
		 * Go to the next group if the player doesn't have the group already, and we can't assign the player to this
		 * group. If we didn't fail the check, then we can assign the player to this group (which means that we need to
		 * set this player as the OwningPlayer for all components in this group).
		 */
		if (!bPlayerAlreadyHasGroup)
		{
			if (!CanAssignOwningPlayerToGroup(PlayerState, Group))
			{
				continue;
			}
		}
		/**
		 * Go to the next group if the player already has a group and the current iterated group is different from the
		 * one the player has. If we didn't fail the check, then we need to set this player as the OwningPlayer for all
		 * components in this group, because some of them may not have the OwningPlayer set yet.
		 */
		else if (GroupNameForPlayer != Group.GroupName)
		{
			continue;
		}

		// Get all components that belong to this group
		TArray<UPlayerOwnershipComponent*> PlayerOwnershipComponents;
		GroupedComponents.MultiFind(Group, PlayerOwnershipComponents);

		// If the player doesn't have a group yet, then we need to find an empty group for him
		if (!bPlayerAlreadyHasGroup)
		{
			bool bGroupHasNoPlayers = true;

			// Make sure that none of the components in this group have the OwningPlayer set yet
			for (const UPlayerOwnershipComponent* PlayerOwnershipComponent : PlayerOwnershipComponents)
			{
#if DO_CHECK
				check(IsValid(PlayerOwnershipComponent));
#endif

				/**
				 * A component from this group already has an OwningPlayer set, so we can stop iterating through the
				 * group.
				 */
				if (PlayerOwnershipComponent->GetOwningPlayer())
				{
					bGroupHasNoPlayers = false;

					break;
				}
			}

			// If the group already has an OwningPlayer set, then we have to skip this group
			if (!bGroupHasNoPlayers)
			{
				continue;
			}

			/**
			 * Initialize the OwningPlayer for all components in this group (we have already checked that they have no
			 * OwningPlayers).
			 */
			for (UPlayerOwnershipComponent* PlayerOwnershipComponent : PlayerOwnershipComponents)
			{
#if DO_CHECK
				check(IsValid(PlayerOwnershipComponent));
#endif

				PlayerOwnershipComponent->InitializeOwningPlayer(PlayerState->GetUniquePlayerID(),
					GetControlledCharacterTypeFromPlayerState(PlayerState));
			}

#if DO_ENSURE
			bFoundGroup = true;
#endif
		}
		/**
		 * Otherwise, we have already found a group this player belongs to above, so we simply need to initialize the
		 * OwningPlayer for the components that don't have it set yet (some of them for sure do).
		 */
		else
		{
			for (UPlayerOwnershipComponent* PlayerOwnershipComponent : PlayerOwnershipComponents)
			{
				// Initialize the OwningPlayer for the component if it doesn't already have one
				if (!PlayerOwnershipComponent->GetOwningPlayer())
				{
					PlayerOwnershipComponent->InitializeOwningPlayer(PlayerState->GetUniquePlayerID(),
						GetControlledCharacterTypeFromPlayerState(PlayerState));
				}
			}

#if DO_ENSURE
			bFoundGroup = true;
#endif
		}

		/**
		 * We found a group for the given player and initialized everything we could, so we can break the loop now.
		 * P.S.: If we get here without assigning the player to the group, then it's an exception.
		 */
		break;
	}

#if DO_ENSURE
	ensureAlwaysMsgf(bFoundGroup,
		TEXT("No group was found for player %s! Consider adding more groups to the level that this player can "
			"be assigned to"),
		*PlayerState->GetPlayerName());
#endif
}

EControlledCharacterType UPlayerOwnershipComponent::GetControlledCharacterTypeFromPlayerState(
	const AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

#if DO_ENSURE
	ensureAlways(!PlayerState->IsSpectator());
#endif

	return PlayerState->IsABot() ? EControlledCharacterType::Bot : EControlledCharacterType::RealPlayer;
}

bool UPlayerOwnershipComponent::CanAssignOwningPlayerToGroup(const AEscapeChroniclesPlayerState* OwningPlayerState,
	const FPlayerOwnershipComponentGroup& Group)
{
#if DO_CHECK
	check(IsValid(OwningPlayerState));
#endif

#if DO_ENSURE
	ensureAlways(!OwningPlayerState->IsSpectator());
#endif

	// Check if this type of player is allowed to be set as the OwningPlayer for this component
	return Group.GroupSettings->AllowedControlledCharacterTypes.Contains(
		GetControlledCharacterTypeFromPlayerState(OwningPlayerState));
}

void UPlayerOwnershipComponent::InitializeOwningPlayer(const FUniquePlayerID& NewOwningPlayer,
	const EControlledCharacterType ControlledCharacterType)
{
#if DO_ENSURE
	// Make sure owning player is initialized only once
	ensureAlways(!OwningPlayerContainer.OwningPlayer.IsValid());
	ensureAlways(OwningPlayerContainer.ControlledCharacterType == EControlledCharacterType::None);

	ensureAlways(NewOwningPlayer.IsValid());

	const FPlayerOwnershipComponentGroup Group = GetGroup();

#if DO_CHECK
	check(Group.GroupSettings);
#endif

	ensureAlwaysMsgf(Group.GroupSettings->AllowedControlledCharacterTypes.Contains(ControlledCharacterType),
		TEXT("The given ControlledCharacterType is not allowed to be set as the OwningPlayer for this "
			"component instance!"));
#endif

	OwningPlayerContainer.OwningPlayer = NewOwningPlayer;

	// Broadcast the delegate
	OnOwningPlayerInitialized.Broadcast(this, OwningPlayerContainer.OwningPlayer,
		OwningPlayerContainer.ControlledCharacterType);
}

void UPlayerOwnershipComponent::CallOrRegister_OnOwningPlayerInitialized(
	const FOnOwningPlayerInitializedDelegate::FDelegate& Callback)
{
	if (OwningPlayerContainer.OwningPlayer.IsValid())
	{
		Callback.ExecuteIfBound(this, OwningPlayerContainer.OwningPlayer, OwningPlayerContainer.ControlledCharacterType);
	}
	else
	{
		OnOwningPlayerInitialized.Add(Callback);
	}
}

void UPlayerOwnershipComponent::OnPostLoadObject()
{
	// Broadcast the delegate if we loaded a valid OwningPlayer
	if (OwningPlayerContainer.OwningPlayer.IsValid())
	{
		OnOwningPlayerInitialized.Broadcast(this, OwningPlayerContainer.OwningPlayer,
			OwningPlayerContainer.ControlledCharacterType);
	}
}