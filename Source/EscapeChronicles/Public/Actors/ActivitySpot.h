// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "Interfaces/Saveable.h"
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
class ESCAPECHRONICLES_API AActivitySpot : public AActor, public IAbilitySystemInterface, public ISaveable
{
	GENERATED_BODY()
	
public:
	AActivitySpot();

#if WITH_EDITORONLY_DATA && WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	/**
	 * @return True if the spot is currently occupied either by a character or another actor. False otherwise.
	 */
	bool IsOccupied() const
	{
		return CachedOccupyingCharacter || !CurrentOccupyingActorClass.IsNull();
	}

	int32 GetGameplayEffectLevel() const { return EffectLevel; }
	void SetGameplayEffectLevel(const int32 InEffectLevel) { EffectLevel = InEffectLevel; }

	// Resets the gameplay effect level to the default value defined in the class defaults
	void ResetGameplayEffectLevel()
	{
		SetGameplayEffectLevel(GetClass()->GetDefaultObject<AActivitySpot>()->GetGameplayEffectLevel());
	}

	/**
	 * Sets the occupying character for this activity spot. If the spot is already occupied by another character, the
	 * operation will fail. If the spot is occupied by an actor other than a character, it will be destroyed.
	 * @param Character The character to occupy the spot (nullptr to clear occupation).
	 * @return True if the occupation state was changed (occupied or unoccupied), false if the spot is already occupied
	 * by another character or no change is needed.
	 * @remark This function will work only on server. Clients' calls will be ignored.
	 */
	bool SetOccupyingCharacter(AEscapeChroniclesCharacter* Character);

	AEscapeChroniclesCharacter* GetOccupyingCharacter() const { return CachedOccupyingCharacter; }

	/**
	 * Gets the ability system component from the occupying character
	 * @return The ASC of occupying character or nullptr if none
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/**
	 * Returns a class of the actor that occupies the spot. Could either be a class of the character, another spawned
	 * actor, or null if the spot is not occupied.
	 * @remark A class isn't guaranteed to be valid and loaded.
	 */
	void GetOccupyingActorClass(TSoftClassPtr<AActor>& OutOccupyingActorClass) const;

	/**
	 * Spawns an actor of the given class and sets it as an occupying actor.
	 * @param OccupyingActorClass A class of the actor to spawn and set as an occupying actor.
	 * @param ActorTransformOnOccupy A relative transform that will be applied to the spawned actor after an attachment.
	 * If nullptr, then the transform for the character attachment will be used.
	 * @return Whether an occupation was successful. If the spot is already occupied, the function will return false.
	 * @return False if the function was called on a client.
	 * @remark This function will not work if the spot is already occupied by anything.
	 * @remark The given actor should be replicated. It will be spawned only on server. Clients' calls of this function
	 * will be ignored.
	 */
	bool SpawnOccupyingActor(const TSoftClassPtr<AActor>& OccupyingActorClass,
		const FTransform* ActorTransformOnOccupy = nullptr);

	/**
	 * Destroys an occupying actor unless it's a character.
	 * @remark This function will work only on server. Clients' calls won't take any effect and will only waste
	 * performance.
	 */
	void DestroyOccupyingActor();

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOccupyingCharacterChanged, AEscapeChroniclesCharacter* NewCharacter);

	// Called when occupation state changes (both occupy and unoccupy)
	FOnOccupyingCharacterChanged OnOccupyingStateChanged;

	virtual void OnPreLoadObject() override;
	virtual void OnPostLoadObject() override;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Handles the actual occupation process (Does not change the value of CachedOccupyingCharacter)
	 * @param Character The character to occupy the spot
	 */
	virtual void OccupySpotWithCharacter(AEscapeChroniclesCharacter* Character);

	/**
	 * Handles cleanup when a character leaves the spot (Does not change the value of CachedOccupyingCharacter)
	 * @param Character The character that is unoccupying
	 */
	virtual void UnoccupySpotWithCharacter(AEscapeChroniclesCharacter* Character);

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
	FTransform CharacterAttachTransform;

	/**
	 * Restores character to cached state
	 * @param SkeletalMesh The character component to restore
	 */
	static void ApplyInitialCharacterData(AEscapeChroniclesCharacter* SkeletalMesh);

	/**
	 * An actual implementation of the SpawnOccupyingActor function. This version doesn't check if the class is valid,
	 * the spot isn't occupied, or if the function is called on a client.
	 */
	bool SpawnOccupyingActorChecked(const TSoftClassPtr<AActor>& OccupyingActorClass,
		const FTransform* ActorTransformOnOccupy = nullptr);

	/**
	 * A class of the actor that currently occupies the spot, if any. Doesn't point to a character's class, only points
	 * to a class of an actor given via the SpawnOccupyingActor function.\n
	 * This class pointer is replicated to support IsOccupied function on clients.\n
	 * This class pointer is saved, and an actor of this class, if it was loaded, will be spawned after it's loaded. 
	 */
	UPROPERTY(Transient, Replicated, SaveGame)
	TSoftClassPtr<AActor> CurrentOccupyingActorClass;

	/**
	 * This property only exists to be able to spawn an actor occupying the spot at the same transform as it was before
	 * it was saved.
	 */
	UPROPERTY(Transient, SaveGame)
	FTransform SpawnedOccupyingActorTransform;

	TSharedPtr<FStreamableHandle> LoadOccupyingActorClassHandle;

	// Spawns an actor attached to the spot at the given transform
	void OnOccupyingActorClassLoaded(const FTransform SpawnTransform);

	/**
	 * An actor that currently occupies the spot. Doesn't point to a character, only points to an actor spawned via the
	 * SpawnOccupyingActor function.
	 */
	TWeakObjectPtr<AActor> SpawnedOccupyingActor;

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
