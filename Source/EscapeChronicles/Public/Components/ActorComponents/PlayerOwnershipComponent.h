// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/Enums/ControlledCharacterType.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Saveable.h"
#include "Common/Structs/UniquePlayerID.h"
#include "PlayerOwnershipComponent.generated.h"

enum class EControlledCharacterType : uint8;

/**
 * The group of the UPlayerOwnershipComponent. All components of the same group share the same settings and must share
 * the same OwningPlayer.
 */
USTRUCT()
struct FPlayerOwnershipComponentGroupTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * What type of player can be an OwningPlayer for this component. If the player doesn't match the given value, then
	 * you must not set him as the OwningPlayer.
	 */
	UPROPERTY(EditAnywhere)
	TSet<EControlledCharacterType> AllowedControlledCharacterTypes;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

// Internal struct for the UPlayerOwnershipComponent that holds the OwningPlayer
USTRUCT()
struct FPlayerOwnershipComponentOwningPlayerContainer
{
	GENERATED_BODY()

	// Player that owns an actor that owns this component
	UPROPERTY(Transient, SaveGame)
	FUniquePlayerID OwningPlayer;

	// The type of the player that owns this component
	UPROPERTY(Transient, SaveGame)
	EControlledCharacterType ControlledCharacterType = EControlledCharacterType::None;
};

/**
 * This component adds ownership of the player to the actor that owns this component. The ownership is represented by an
 * FUniquePlayerID that is set on the component.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UPlayerOwnershipComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerOwnershipComponent()
	{
		PrimaryComponentTick.bCanEverTick = false;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Should be used to filter which OwningPlayer to set on the component.
	 * @return The pair that contains the group name and the group settings. All components with the same group name
	 * must have the same OwningPlayer. The group settings must be used to determine whether you can set the player as
	 * an OwningPlayer or not.
	 * @remark The group settings shouldn't be null unless the component was set up wrong.
	 */
	TPair<FName, FPlayerOwnershipComponentGroupTableRow*> GetGroup() const;

	/**
	 * @return OwningPlayer if it's valid. Otherwise, nullptr.
	 * @remark Consider using CallOrRegister_OnOwningPlayerInitialized to guarantee that you get a valid OwningPlayer.
	 */
	const FUniquePlayerID* GetOwningPlayer() const
	{
		return OwningPlayerContainer.OwningPlayer.IsValid() ? &OwningPlayerContainer.OwningPlayer : nullptr;
	}

	/**
	 * @return The type of the player that owns this component if it's valid. Otherwise, EControlledCharacterType::None.
	 * @remark Consider using CallOrRegister_OnOwningPlayerInitialized to guarantee that you get a valid value.
	 */
	EControlledCharacterType GetControlledCharacterType() const
	{
		return OwningPlayerContainer.ControlledCharacterType;
	}

	/**
	 * Should be used by the GameMode when the new player is connected to the game or a bot is spawned. All components
	 * that share the same group must have the same owning player. The only exclusion is when the group is empty, in
	 * which case there should be no owning player. Also, the group may have its own rules for setting the owning
	 * player, which you must follow.
	 * @param NewOwningPlayer An ID of the owning player.
	 * @param ControlledCharacterType A type of the owning player.
	 */
	void InitializeOwningPlayer(const FUniquePlayerID& NewOwningPlayer,
		const EControlledCharacterType ControlledCharacterType);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnOwningPlayerInitializedDelegate, const FUniquePlayerID& OwningPlayer,
		EControlledCharacterType ControlledCharacterType);

	/**
	 * Calls the given callback if OwningPlayer is already initialized or registers the callback to be called when the
	 * OwningPlayer is initialized.
	 */
	void CallOrRegister_OnOwningPlayerInitialized(const FOnOwningPlayerInitializedDelegate::FDelegate& Callback);

private:
	/**
	 * The group of this component instance. All components' instances of the same group share the same settings and
	 * must share the same OwningPlayer.
	 * @remark You must use the same DataTable for all components on the level.
	 */
	UPROPERTY(EditInstanceOnly, meta=(RequiredAssetDataTags="RowStructure=PlayerOwnershipComponentGroupTableRow"))
	FDataTableRowHandle Group;

	// Data about the player that owns the actor that owns this component
	UPROPERTY(Transient, Replicated, SaveGame)
	FPlayerOwnershipComponentOwningPlayerContainer OwningPlayerContainer;

	// Called when the OwningPlayer is set
	FOnOwningPlayerInitializedDelegate OnOwningPlayerInitialized;
};