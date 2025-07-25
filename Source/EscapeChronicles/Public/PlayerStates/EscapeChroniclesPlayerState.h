// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"
#include "Common/Structs/UniquePlayerID.h"
#include "EscapeChroniclesPlayerState.generated.h"

class UAbilitySystemSet;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesPlayerState : public APlayerState, public IAbilitySystemInterface,
	public ISaveable
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

	// The same as AController::InitPlayerState() but with a custom PlayerState class
	static void InitPlayerStateForController(AController* OwnerController,
		const TSubclassOf<AEscapeChroniclesPlayerState>& PlayerStateClass);

	virtual void PostInitializeComponents() override;

	bool IsOnlinePlayer() const;

	/**
	 * The PlayerID in the struct should be overriden by the SaveGameSubsystem if it found the save for the same NetId
	 * but their PlayerIDs are different.
	 */
	FUniquePlayerID& GetUniquePlayerID_Mutable() { return UniquePlayerID; }

	const FUniquePlayerID& GetUniquePlayerID() const { return UniquePlayerID; }

	void GenerateUniquePlayerIdIfInvalid();

	virtual bool CanBeSavedOrLoaded() const override { return !IsSpectator(); }

protected:
	UFUNCTION()
	virtual void OnPawnChanged(APlayerState* ThisPlayerState, APawn* NewPawn, APawn* OldPawn);

	/**
	 * Called when the new pawn is set (except of the situation when you switch between spectator pawns), but before the
	 * gameplay effects from AbilitySystemSets are applied. This could be overriden in child classes to initialize
	 * attributes with custom data, if any.
	 * @remark The suggested way to initialize attributes is to use gameplay effects. This function exists only for
	 * attributes that have to be initialized with data that already exists in other classes (for example, in the pawn
	 * or his components).
	 */
	virtual void InitializeAttributes() {}

private:
	UPROPERTY(VisibleAnywhere, Category="Ability System")
	TObjectPtr<UEscapeChroniclesAbilitySystemComponent> AbilitySystemComponent;

	// Ability system sets to grant to this pawn's ability system
	UPROPERTY(EditDefaultsOnly, Category = "Ability System|Abilities")
	TArray<TObjectPtr<UAbilitySystemSet>> AbilitySystemSets;

	TWeakObjectPtr<APawn> LastNotSpectatorPawn;

	UPROPERTY(Transient, Replicated)
	FUniquePlayerID UniquePlayerID;
};