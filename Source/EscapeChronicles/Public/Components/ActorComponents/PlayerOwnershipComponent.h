// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Structs/UniquePlayerID.h"
#include "Interfaces/Saveable.h"
#include "PlayerOwnershipComponent.generated.h"

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
	const FGuid& GetConnectedComponentsID() const { return ConnectedComponentsID; }

	/**
	 * @return OwningPlayer if it's valid. Otherwise, nullptr.
	 */
	const FUniquePlayerID* GetOwningPlayer() const
	{
		return OwningPlayer.IsValid() ? &OwningPlayer : nullptr;
	}

	/**
	 * Should be used by the GameMode when the new player is connected to the game or a bot is spawned. All components
	 * that share the same ConnectedComponentsID must have the same owning player. The only exclusion is when the
	 * ConnectedComponentsID is empty, in which case there should be no owning player.
	 */
	void SetOwningPlayer(const FUniquePlayerID& NewOwningPlayer) { OwningPlayer = NewOwningPlayer; }

private:
	// ID of the UPlayerOwnershipComponents that should share the same owning player
	UPROPERTY(EditInstanceOnly)
	FGuid ConnectedComponentsID;

	// Player that owns an actor that owns this component
	UPROPERTY(Transient, Replicated, SaveGame)
	FUniquePlayerID OwningPlayer;
};