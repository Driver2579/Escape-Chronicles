// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Interfaces/Saveable.h"
#include "Actors/ActivitySpot.h"
#include "Common/Structs/UniquePlayerID.h"
#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"
#include "EscapeChroniclesPlayerState.generated.h"

class UTeamIDsSet;
class UAbilitySystemSet;

enum class ECharacterRole : uint8;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesPlayerState : public APlayerState, public IAbilitySystemInterface,
	public ISaveable, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AEscapeChroniclesPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override final
	{
		return AbilitySystemComponent;
	}

	UEscapeChroniclesAbilitySystemComponent* GetEscapeChroniclesAbilitySystemComponent() const
	{
		return AbilitySystemComponent;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	AActivitySpot* GetOccupyingActivitySpot() const { return OccupyingActivitySpot.Get(); }

	void SetOccupyingActivitySpot(AActivitySpot* InOccupyingActivitySpot)
	{
		OccupyingActivitySpot = InOccupyingActivitySpot;
	}

	// The same as AController::InitPlayerState() but with a custom PlayerState class
	static void InitPlayerStateForController(AController* OwnerController,
		const TSubclassOf<AEscapeChroniclesPlayerState>& PlayerStateClass);

	virtual void PostInitializeComponents() override;

	bool IsOnlinePlayer() const;

	const FUniquePlayerID& GetUniquePlayerID() const { return UniquePlayerID; }

	// Should be used only by the SaveGameSubsystem
	void SetUniquePlayerID(const FUniquePlayerID& NewUniquePlayerID);

	void GenerateUniquePlayerIdIfInvalid();

	// Returns the role depending on the gameplay tags
	ECharacterRole GetCharacterRole() const;

	virtual bool CanBeSavedOrLoaded() const override { return !IsSpectator(); }

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override { TeamID = NewTeamID; }

protected:
	/**
	 * Called from the GenerateUniquePlayerIdIfInvalid function if it generates the new UniquePlayerID and from the
	 * SetUniquePlayerID function.
	 */
	virtual void OnUniquePlayerIdInitializedOrChanged() {}

private:
	UPROPERTY(VisibleAnywhere, Category="Ability System")
	TObjectPtr<UEscapeChroniclesAbilitySystemComponent> AbilitySystemComponent;

	// Ability system sets to grant to this pawn's ability system
	UPROPERTY(EditDefaultsOnly, Category = "Ability System|Abilities")
	TArray<TObjectPtr<UAbilitySystemSet>> AbilitySystemSets;

	UPROPERTY(Transient, Replicated)
	FUniquePlayerID UniquePlayerID;

	TWeakObjectPtr<AActivitySpot> OccupyingActivitySpot;

	// The set of team IDs that the player will use to determine its team
	UPROPERTY(EditDefaultsOnly, Category="Team")
	TObjectPtr<UTeamIDsSet> TeamIDsSet;

	FGenericTeamId TeamID;
};