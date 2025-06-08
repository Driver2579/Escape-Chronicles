// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystem/AttributeSets/CombatAttributeSet.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "GameFramework/Actor.h"
#include "ActivitySpot.generated.h"

class AEscapeChroniclesPlayerState;
class UPlayerOwnershipComponent;
struct FStreamableHandle;
class AEscapeChroniclesCharacter;
class UInteractableComponent;
class UGameplayEffect;

// Actor is a place with which the character can interact and receive an effect
UCLASS()
class ESCAPECHRONICLES_API AActivitySpot : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	// When a character occupies or unoccupies this actor this delegate is triggered
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOccupyingCharacterChanged, AEscapeChroniclesCharacter* Character);
	
	AActivitySpot();

	/**
	 * Determines who is currently occupying
	 * @param Character The one who wants to occupy, or nullptr to unoccupy current character
	 * @return True on success, false otherwise
	 */
	bool SetOccupyingCharacter(AEscapeChroniclesCharacter* Character);

	AEscapeChroniclesCharacter* GetOccupyingCharacter() const { return CachedOccupyingCharacter; }
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void AddOccupyingCharacterChangedHandler(const FOnOccupyingCharacterChanged::FDelegate& Callback);
	
protected:
	virtual void BeginPlay() override;

	/**
	 * Tries to set the initialized character as an occupying character for this spot if
	 * bOccupyWhenOwningPlayerFirstJoined is true, the initialized player or bot wasn't loaded (which means that he
	 * joined the game for the first time), and it is the player or bot that owns this spot.
	 */
	virtual void OnPlayerOrBotInitialized(AEscapeChroniclesPlayerState* PlayerState, const bool bLoaded);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Makes character occupy this actor (The value of CachedOccupyingCharacter does not change)
	virtual void OccupySpot(AEscapeChroniclesCharacter* Character);

	// Makes character unoccupy this actor (The value of CachedOccupyingCharacter does not change)
	virtual void UnoccupySpot(AEscapeChroniclesCharacter* Character);
	
private:
	FOnOccupyingCharacterChanged OnOccupyingCharacterChanged;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UPlayerOwnershipComponent> PlayerOwnershipComponent;

	// === Attaching ===
	
	// Mesh for display, interaction and attach to when character occupying
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Socket on the mesh that the character will attach to when it occupying 
	UPROPERTY(EditAnywhere)
	FName AttachSocketName;
	
	// === Interaction ===
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInteractableComponent> InteractableComponent;
	
	void OnInteract(UInteractionManagerComponent* InteractionManagerComponent);
    
	// === Effect ===

	// Effect that is applyed on the occupied character and is removed when he gets up.
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameplayEffect> OccupingEffectClass;

	UPROPERTY(EditAnywhere)
	int32 EffectLevel;

	FActiveGameplayEffectHandle OccupingEffectSpecHandle;

	// === Animation montage ===
    
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UAnimMontage>> OccupingAnimMontages;

	int32 SelectedOccupingAnimMontage;
	
	// === Occupying ===
	
	UPROPERTY(ReplicatedUsing="OnRep_CachedOccupyingCharacter")
	TObjectPtr<AEscapeChroniclesCharacter> CachedOccupyingCharacter;
	
	UFUNCTION()
	void OnRep_CachedOccupyingCharacter(AEscapeChroniclesCharacter* OldValue);

	// OccupySpot does not work on characters with these tags
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer OccupyingBlockedTags;
	
	FTransform CachedMeshTransform;
	TWeakObjectPtr<USceneComponent> CachedMeshAttachParent;

	// If true, then the owning player or bot will occupy this spot when he first joined the game (i.e., wasn't loaded)
	UPROPERTY(EditAnywhere)
	bool bOccupyWhenOwningPlayerFirstJoined = false;

	// === Events when unoccupy occupied character ===

	// When health decreases
	void OnOccupyingCharacterHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
	FDelegateHandle UnoccupyIfAttributeHasDecreasedDelegateHandle;

	// === Asset loading ===

	void OnOccupyingAnimMontageLoaded();
	void OnOccupyingEffectLoaded();

	TSharedPtr<FStreamableHandle> OccupyingAnimMontageHandle;
	TSharedPtr<FStreamableHandle> OccupyingEffectHandle;

	void CancelOccupyingAnimationAndEffect(AEscapeChroniclesCharacter* Character);
};
