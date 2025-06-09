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

	int32 GetEffectLevel() const { return EffectLevel; }

	void SetEffectLevel(const int32 InEffectLevel) { EffectLevel = InEffectLevel; }

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

	// Broadcast when occupation state changes (both occupy and unoccupy)
	FOnOccupyingCharacterChanged OnOccupyingCharacterChanged;

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

	// Character's location after unoccupy
	UPROPERTY()
	FVector CharacterLocationOnOccupySpot;

	// Character's rotation after unoccupy
	UPROPERTY()
	FRotator CharacterRotationOnOccupySpot;

	// Mesh socket name for character attachment during occupation
	UPROPERTY(EditAnywhere)
	FName AttachSocketName;

	/**
	 * Saves character's mesh transform data for later restoration
	 * @param SkeletalMesh The mesh component to cache
	 */
	void CacheMeshData(const USkeletalMeshComponent* SkeletalMesh);

	/**
	 * Restores character's mesh to cached transform
	 * @param SkeletalMesh The mesh component to restore
	 */
	void ApplyCachedMeshData(USkeletalMeshComponent* SkeletalMesh) const;

	/**
	 * Attach character's mesh to MeshComponent
	 * @param SkeletalMesh The mesh component to attach
	 */
	void AttachSkeletalMesh(USkeletalMeshComponent* SkeletalMesh) const;

	void AttachOccupyingCharacterMesh() const;

	// Original transform of character's mesh before occupation
	FTransform CachedMeshTransform;

	// Original parent of character's mesh before occupation
	TWeakObjectPtr<USceneComponent> CachedMeshAttachParent;

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
