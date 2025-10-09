// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Saveable.h"
#include "GameplayTagContainer.h"
#include "Door.generated.h"

class AEscapeChroniclesCharacter;
class UBoxComponent;
class UPhysicsConstraintComponent;

/**
 * Door actor with configurable access rules (keys/tags) and automatic collision handling.
 * Access checks are performed separately for entering and exiting sides.
 * The door also handles key usage (durability reduction)
 * That prevents passage unless conditions are met. Access can be granted by:
 *  - possession of a matching key (defined by KeyAccessTag),
 *  - having a character access tag from CharacterAccessTags,
 *  - or when key requirement is disabled (bEnterRequiresKey / bExitRequiresKey).
 */
UCLASS()
class ESCAPECHRONICLES_API ADoor : public AActor, public ISaveable
{
	GENERATED_BODY()

public:	
	ADoor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const FGameplayTag& GetKeyAccessTag() const { return KeyAccessTag; }
	const FGameplayTagContainer& GetCharacterAccessTags() const { return CharacterAccessTags; }

	bool DoesEnterRequireKey() const { return bEnterRequiresKey; }

	void SetEnterRequiresKey(const bool bNewEnterRequiresKey )
	{
		bEnterRequiresKey = bNewEnterRequiresKey ;

		UpdateConfirmedCharactersPool();
	}
	
	bool DoesExitRequireKey() const { return bExitRequiresKey; }

	void SetExitRequiresKey(const bool bNewExitRequiresKey)
	{
		bExitRequiresKey = bNewExitRequiresKey;

		UpdateConfirmedCharactersPool();
	}
	
	/**
	 * @return True if character's ASC contains an access tag for this door.
	 */
	bool DoesCharacterHaveAccessTag(const AEscapeChroniclesCharacter* Character) const;

	/**
	 * @return True if character's inventory contains a matching key (the key fragment's tag matches the door's tag).
	 */
	bool DoesCharacterHaveMatchingKey(const AEscapeChroniclesCharacter* Character) const;

	/**
	 * Checks if the given character is allowed to enter through this door.
	 * A character has enter access if one of the conditions is met:
	 *  - entering currently does not require a key (bEnterRequiresKey == false).
	 *  - the character has one of the CharacterAccessTags.
	 *  - the character possesses a key with a tag matching KeyAccessTag.
	 */
	bool DoesCharacterHaveEnterAccess(const AEscapeChroniclesCharacter* Character) const
	{
		return !bEnterRequiresKey || DoesCharacterHaveAccessTag(Character) || DoesCharacterHaveMatchingKey(Character);
	}

	/**
	 * Checks if the given character is allowed to exit through this door.
	 * A character has exit access if one of the conditions is met:
	 *  - exiting currently does not require a key (bExitRequiresKey == false).
	 *  - the character has one of the CharacterAccessTags.
	 *  - the character possesses a key with a tag matching KeyAccessTag.
	 */
	bool DoesCharacterHaveExitAccess(const AEscapeChroniclesCharacter* Character) const
	{
		return !bExitRequiresKey || DoesCharacterHaveAccessTag(Character) || DoesCharacterHaveMatchingKey(Character);
	}

	virtual void OnPostLoadObject() override
	{
		UpdateConfirmedCharactersPool();
	}

protected:
	virtual void BeginPlay() override;

private:
	/* 
	 * Entrance trigger zone. Must be placed only on one door side and MUST NOT overlap with ExitBoxComponent.
	 * Character presence in this zone means he's on the outside of the door.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> EnterBoxComponent;

	/*
	 * Exit trigger zone. Must be placed only on one door side and MUST NOT overlap with EnterBoxComponent.
	 * Character presence in this zone means he's on the inside of the door
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> ExitBoxComponent;

	/**
	 * Overlap zone used to determine whether the door should be unlocked for a character. Must be placed between
	 * EnterBoxComponent and ExitBoxComponent. When a valid access condition is met, SetLockDoorway() is called to
	 * toggle the DoorwayBlockingComponent collider and allow passage through the doorway.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxZoneComponent;

	/*
	 * Physical door blocker collider. Blocks passage when active. Automatically disabled during valid access. Should
	 * fully cover doorway and be inside the DoorwayBoxOverlap. Automatically disabled for specific character while he
	 * overlaps the door and has valid access to go through it.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBlockingComponent;

	/**
	 * Access tag required to open this door. A character can pass if they possess an inventory item with a key fragment
	 * whose tag matches this KeyAccessTag.
	 */
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTag KeyAccessTag;

	/**
	 * Characters that have one of these tags can pass freely through the door even if the door is closed and the
	 * character doesn't have a key.
	 */
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTagContainer CharacterAccessTags;

	// Whether the character need to use a key to enter this door
	UPROPERTY(EditAnywhere, Category="Access", Replicated, SaveGame)
	bool bEnterRequiresKey = false;

	// Whether the character need to use a key to exit this door
	UPROPERTY(EditAnywhere, Category="Access", Replicated, SaveGame)
	bool bExitRequiresKey = false;

	UFUNCTION()
	virtual void OnDoorwayBoxZoneComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnDoorwayBoxZoneComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	virtual void OnEnterOrExitBoxComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	/**
	 * Enables or disables the doorway blocker for the given character. Called during access checks to unlock
	 * (IsLock = false) or lock (IsLock = true) the physical passage through the door.
	 */
	void SetLockDoorway(AEscapeChroniclesCharacter* Character, bool IsLock) const;
	
	/**
	 * Reduces the durability of the key item held by the given character by 1, if applicable. Only keys with a Key
	 * fragment that has durability enabled (uses durability) and whose tag matches this door's KeyAccessTag will be
	 * affected. Called when the character uses the key to unlock the door.
	 */
	void UseKey(const AEscapeChroniclesCharacter* Character) const;

	/**
	 * Updates the pool of characters who are currently passing through the door. Ensures that characters are tracked
	 * correctly for access and locking logic.
	 */
	void UpdateConfirmedCharactersPool();

	/**
	 * Attempts to add a character to the confirmed pool if they meet access conditions. Used internally to track
	 * characters actively passing through the door.
	 */
	void TryAddCharacterToPool(AEscapeChroniclesCharacter* Character);

	/**
	 * Characters who are currently in the process of passing through the door.
	 * This pool is used to track characters during overlap events to ensure that:
	 *  - access checks are consistently applied,
	 *  - the doorway blocker (DoorwayBlockingComponent) is correctly locked/unlocked,
	 *  - key usage (durability reduction) is applied only once per passage.
	 * Characters are added via TryAddCharacterToPool() and updated with UpdateConfirmedCharactersPool().
	 */
	TArray<TWeakObjectPtr<AEscapeChroniclesCharacter>> ConfirmedCharactersPool;
};
