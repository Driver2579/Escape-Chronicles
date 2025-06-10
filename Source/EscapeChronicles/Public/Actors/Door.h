// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

class AEscapeChroniclesCharacter;
class UBoxComponent;
class UPhysicsConstraintComponent;

// Door actor with configurable access rules (keys/tags) and automatic collision handling
UCLASS()
class ESCAPECHRONICLES_API ADoor : public AActor
{
	GENERATED_BODY()

public:	
	ADoor();

	const FGameplayTag& GetKeyAccessTag() const { return KeyAccessTag; }
	const FGameplayTagContainer& GetCharacterAccessTags() const { return CharacterAccessTags; }
	bool IsEnterRequiresKey() const { return bEnterRequiresKey; }
	bool IsExitRequiresKey() const { return bExitRequiresKey; }
	
	// Returns true if the character has access tag to this door
	bool HasCharacterAccessTag(const AEscapeChroniclesCharacter* Character) const;
	
	// Returns true if the character has a matching key
	bool HasCharacterMatchingKey(const AEscapeChroniclesCharacter* Character) const;

	bool HasCharacterEnterAccess(const AEscapeChroniclesCharacter* Character) const
	{
		return !bEnterRequiresKey || HasCharacterAccessTag(Character) || HasCharacterMatchingKey(Character);
	}
	
	bool HasCharacterExitAccess(const AEscapeChroniclesCharacter* Character) const
	{
		return !bExitRequiresKey || HasCharacterAccessTag(Character) || HasCharacterMatchingKey(Character);
	}

	void SetEnterRequiresKey(const bool InEnterRequiresKey)
	{
		bEnterRequiresKey = InEnterRequiresKey;

		UpdateConfirmedCharactersPool();
	}
	
	void SetExitRequiresKey(const bool InExitRequiresKey)
	{
		bExitRequiresKey = InExitRequiresKey;

		UpdateConfirmedCharactersPool();
	}

protected:
	virtual void BeginPlay() override;

private:
	/* 
	 * Entrance trigger zone. Must be placed only on one door side and MUST NOT overlap with ExitBox.
	 * Character presence in this zone means they're on the outside of the door.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> EnterBox;

	/*
	 * Exit trigger zone. Must be placed only on one door side and MUST NOT overlap with EnterBox.
	 * Character presence in this zone means They're on the inside of the door
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> ExitBox;

	// Checks if the door needs to be unlocked. Must be between EnterBox and ExitBox.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxOverlap;

	/*
	 * Physical door blocker collider. Blocks passage when active. Automatically disabled during valid access. Should
	 * fully cover doorway and be inside the DoorwayBoxOverlap.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxBlock;

	// Tag that the key must have to open this door
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTag KeyAccessTag;

	// Characters who have one of these tags can pass freely through the door
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTagContainer CharacterAccessTags;

	// Do character need to use a key to enter this door
	UPROPERTY(EditAnywhere, Category="Access")
	bool bEnterRequiresKey = false;

	// Do character need to use a key to exit this door
	UPROPERTY(EditAnywhere, Category="Access")
	bool bExitRequiresKey = false;

	UFUNCTION()
	void OnDoorwayBoxOverlapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDoorwayBoxOverlapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void OnEnterOrExitBoxOverlapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// Returns true if the key is needed in the current position
	bool IsRequiresKey(const AEscapeChroniclesCharacter* Character) const;
	
	// Sets whether a character can pass through a door
	void SetLockDoorway(const AEscapeChroniclesCharacter* Character, bool IsLock) const;
	
	// Removes 1 unit of durability if needed
	void UseKey(const AEscapeChroniclesCharacter* Character) const;

	void UpdateConfirmedCharactersPool();

	void TryAddCharacterToPool(AEscapeChroniclesCharacter* Character);

	// Characters who are in the process of going through the door
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> ConfirmedCharactersPool;
};
