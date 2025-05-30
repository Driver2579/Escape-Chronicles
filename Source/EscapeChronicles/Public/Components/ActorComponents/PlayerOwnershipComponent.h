// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Saveable.h"
#include "GameplayTagContainer.h"
#include "Common/Enums/ControlledCharacterType.h"
#include "Common/Structs/UniquePlayerID.h"
#include "PlayerOwnershipComponent.generated.h"

class AEscapeChroniclesPlayerState;
class UPlayerOwnershipComponent;

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
	 * he can't become an OwningPlayer of this component.
	 */
	UPROPERTY(EditAnywhere)
	TSet<EControlledCharacterType> AllowedControlledCharacterTypes;

	/**
	 * What tags the player must have to become an OwningPlayer of this component. If the player doesn't have any of
	 * these tags, then he can't become an OwningPlayer of this component.
	 */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer RequiredTags;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

// A helper struct for the group of the UPlayerOwnershipComponent
struct FPlayerOwnershipComponentGroup
{
	// The name of the group. All components with the same group name must have the same OwningPlayer.
	FName GroupName;

	/**
	 * The settings of the group. The group settings must be used to determine whether the player can be set as an
	 * OwningPlayer or not.
	 * @remark Shouldn't be null unless the component was set up wrong.
	 */
	FPlayerOwnershipComponentGroupTableRow* GroupSettings = nullptr;

	bool operator==(const FPlayerOwnershipComponentGroup& Other) const
	{
		return GroupName == Other.GroupName;
	}
};

// This is required to use FPlayerOwnershipComponentGroup as a key in TMap and TSet
FORCEINLINE uint32 GetTypeHash(const FPlayerOwnershipComponentGroup& Group)
{
	return FCrc::MemCrc32(&Group, sizeof(Group));
}

// Internal struct for the UPlayerOwnershipComponent that holds the OwningPlayer
USTRUCT()
struct FPlayerOwnershipComponentOwningPlayerContainer
{
	GENERATED_BODY()

	// Player that owns an actor that owns this component
	UPROPERTY(Transient)
	FUniquePlayerID OwningPlayer;

	// The type of the player that owns this component
	UPROPERTY(Transient)
	EControlledCharacterType ControlledCharacterType = EControlledCharacterType::None;
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnOwningPlayerInitializedDelegate,
	UPlayerOwnershipComponent* PlayerOwnershipComponent, const FUniquePlayerID& OwningPlayer,
	const FPlayerOwnershipComponentGroup& Group);

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

	// Should be used to filter which OwningPlayer to set on the component
	FPlayerOwnershipComponentGroup GetGroup() const;

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
	 * Selects the group of UPlayerOwnershipComponents for the given player and sets this player as an owning player for
	 * all UPlayerOwnershipComponents on the scene that share the same group.
	 * @remark This must be called for all not spectating players and bots when they are spawned!
	 * @remark It's best to call it AFTER the game is loaded. Otherwise, you may register the player for the group, and
	 * he may be immediately overriden after loading the game.
	 */
	static void RegisterPlayer(const AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * This function is used by the RegisterPlayer function to initialize the owning player, but it should be called
	 * manually if this component is spawned dynamically. All components that share the same group must have the same
	 * owning player. The only exclusion is when the group is empty, in which case there should be no owning player.
	 * Also, the group may have its own rules for setting the owning player, which you must follow (you can check if the
	 * player matches the rules of the group by using the CanAssignOwningPlayerToGroup function).
	 * @param NewOwningPlayer An ID of the owning player.
	 * @param ControlledCharacterType A type of the owning player.
	 */
	void InitializeOwningPlayer(const FUniquePlayerID& NewOwningPlayer,
		const EControlledCharacterType ControlledCharacterType);

	/**
	 * Calls the given callback if OwningPlayer is already initialized or registers the callback to be called when the
	 * OwningPlayer is initialized.
	 * @return DelegateHandle for the callback. Invalid if the callback was called immediately.
	 */
	FDelegateHandle CallOrRegister_OnOwningPlayerInitialized(
		const FOnOwningPlayerInitializedDelegate::FDelegate& Callback);

	// This should be called when you don't need to listen for the OwningPlayer initialization anymore
	void Unregister_OnOwningPlayerInitialized(const FDelegateHandle& DelegateHandle)
	{
		OnOwningPlayerInitialized.Remove(DelegateHandle);
	}

protected:
	virtual void OnPostLoadObject() override;

private:
	/**
	 * The group of this component instance. All components' instances of the same group share the same settings and
	 * must share the same OwningPlayer.
	 * @remark You must use the same DataTable for all components on the level.
	 */
	UPROPERTY(EditInstanceOnly, DisplayName="Group",
		meta=(RequiredAssetDataTags="RowStructure=PlayerOwnershipComponentGroupTableRow"))
	FDataTableRowHandle GroupData;

	// Data about the player that owns the actor that owns this component
	UPROPERTY(Transient, Replicated)
	FPlayerOwnershipComponentOwningPlayerContainer OwningPlayerContainer;

	static EControlledCharacterType GetControlledCharacterTypeFromPlayerState(
		const AEscapeChroniclesPlayerState* PlayerState);

	/**
	 * Checks if the given player can be set as the OwningPlayer for components from the given group by checking the
	 * group settings.
	 * @reamrk This function doesn't check if there is already an OwningPlayer set for the given group, so you need to
	 * check it manually.
	 */
	static bool CanAssignOwningPlayerToGroup(const AEscapeChroniclesPlayerState* OwningPlayerState,
		const FPlayerOwnershipComponentGroup& Group);

	// Called when the OwningPlayer is set
	FOnOwningPlayerInitializedDelegate OnOwningPlayerInitialized;
};