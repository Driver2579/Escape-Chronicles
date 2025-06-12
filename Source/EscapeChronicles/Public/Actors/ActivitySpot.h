// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "ActivitySpot.generated.h"

struct FOnAttributeChangeData;
class UInteractionManagerComponent;
class UCapsuleComponent;
class UPlayerOwnershipComponent;
struct FStreamableHandle;
class AEscapeChroniclesCharacter;
class UInteractableComponent;
class UGameplayEffect;

/**
 * Interactive spot where characters can perform activities with visual and gameplay effects.
 * This actor provides a location where characters can interact to:
 * - Play specific animations.
 * - Apply gameplay effects.
 * - Auto unoccupy when health decreasing.
 */
UCLASS()
class ESCAPECHRONICLES_API AActivitySpot : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AActivitySpot();

#if WITH_EDITORONLY_DATA && WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	bool IsOccupied() const { return  CachedOccupyingCharacter != nullptr; }

	int32 GetGameplayEffectLevel() const { return EffectLevel; }
	void SetGameplayEffectLevel(const int32 InEffectLevel) { EffectLevel = InEffectLevel; }

	// Resets the gameplay effect level to the default value defined in the class defaults
	void ResetGameplayEffectLevel()
	{
		SetGameplayEffectLevel(GetClass()->GetDefaultObject<AActivitySpot>()->GetGameplayEffectLevel());
	}

	/**
	 * Sets the occupying character for this activity spot. If the spot is already occupied by another character, the
	 * operation will fail. (Server-only)
	 * @param Character The character to occupy the spot (nullptr to clear occupation).
	 * @return True if occupation state was changed (occupied or unoccupied), false if the spot is already occupied by
	 * another character or no change is needed.
	 */
	bool SetOccupyingCharacter(AEscapeChroniclesCharacter* Character);

	AEscapeChroniclesCharacter* GetOccupyingCharacter() const { return CachedOccupyingCharacter; }

	/**
	 * Gets the ability system component from the occupying character
	 * @return The ASC of occupying character or nullptr if none
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOccupyingCharacterChanged, AEscapeChroniclesCharacter* NewCharacter);

	// Called when occupation state changes (both occupy and unoccupy)
	FOnOccupyingCharacterChanged OnOccupyingStateChanged;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Handles the actual occupation process (Does not change the value of CachedOccupyingCharacter)
	 * @param Character The character to occupy the spot
	 */
	virtual void OccupySpot(AEscapeChroniclesCharacter* Character);

	/**
	 * Handles cleanup when character leaves the spot (Does not change the value of CachedOccupyingCharacter)
	 * @param Character The character that is unoccupying
	 */
	virtual void UnoccupySpot(AEscapeChroniclesCharacter* Character);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlayerOwnershipComponent> PlayerOwnershipComponent;

	// === Attaching ===

	// Visual representation and interaction surface for the spot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

#if WITH_EDITORONLY_DATA
	// Visual helper in editor for setting up character positioning after unoccupy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCapsuleComponent> CharacterTransformOnOccupySpotComponent;
#endif

	// Character's transform after unoccupy
	UPROPERTY()
	FTransform CharacterTransformOnOccupySpot;

	// Mesh transform for character attachment during occupation
	UPROPERTY(EditAnywhere)
	FTransform AttachTransform;

	/**
	 * Restores character to cached state
	 * @param SkeletalMesh The character component to restore
	 */
	static void ApplyInitialCharacterData(AEscapeChroniclesCharacter* SkeletalMesh);

	// === Interaction ===

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteractableComponent> InteractableComponent;

	void OnInteract(UInteractionManagerComponent* InteractionManagerComponent);

	// === Effect ===

	// Gameplay effect applied to occupying characters
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameplayEffect> OccupyingEffectClass;

	// Initial occupying effect level
	UPROPERTY(EditAnywhere)
	int32 EffectLevel;

	// Handle to the active gameplay effect instance
	FActiveGameplayEffectHandle OccupyingEffectSpecHandle;

	// === Animation montage ===

	// Possible animations to play when character occupies the spot 
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UAnimMontage>> OccupyingAnimMontages;

	// Index of currently selected animation montage
	int32 SelectedOccupyingAnimMontage;

	// === Occupying ===

	// Currently occupying character
	UPROPERTY(ReplicatedUsing="OnRep_CachedOccupyingCharacter")
	TObjectPtr<AEscapeChroniclesCharacter> CachedOccupyingCharacter;

	/**
	 * Replication callback for occupying character changes
	 * @param OldValue The previous occupying character
	 */
	UFUNCTION()
	void OnRep_CachedOccupyingCharacter(AEscapeChroniclesCharacter* OldValue);

	// Gameplay tags that prevent occupation
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer OccupyingBlockedTags;

	// === Health Tracking ===

	void OnOccupyingCharacterHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
	FDelegateHandle UnoccupyIfAttributeHasDecreasedDelegateHandle;

	// === Asset loading ===

	void LoadOccupyingAnimMontage();
	void LoadOccupyingEffect();

	void OnOccupyingAnimMontageLoaded();
	void OnOccupyingEffectLoaded();

	TSharedPtr<FStreamableHandle> OccupyingAnimMontageHandle;
	TSharedPtr<FStreamableHandle> OccupyingEffectHandle;

	void CancelOccupyingAnimation(const USkeletalMeshComponent* CharacterMesh);
	void CancelOccupyingEffect(const AEscapeChroniclesCharacter* Character);
};
