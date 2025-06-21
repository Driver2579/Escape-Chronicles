// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Saveable.h"
#include "Door.generated.h"

class AEscapeChroniclesCharacter;
class UBoxComponent;
class UPhysicsConstraintComponent;

// Door actor with configurable access rules (keys/tags) and automatic collision handling
UCLASS()
class ESCAPECHRONICLES_API ADoor : public AActor, public ISaveable
{
	GENERATED_BODY()

public:	
	ADoor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	}
	
	void SetExitRequiresKey(const bool InExitRequiresKey)
	{
		bExitRequiresKey = InExitRequiresKey;
	}

	/**
	 * TODO: Implement UpdateConfirmedCharactersPool(); and call it in methods: OnPostLoadObject, SetEnterRequiresKey,
	 * SetExitRequiresKey.
	 */

protected:
	virtual void BeginPlay() override;

private:
	/* 
	 * Entrance trigger zone. Must be placed only on one door side and MUST NOT overlap with ExitBox, only touch.
	 * Character presence in this zone means they're on the outside of the door.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> EnterBox;

	/*
	 * Exit trigger zone. Must be placed only on one door side and MUST NOT overlap with EnterBox, only touch.
	 * Character presence in this zone means They're on the inside of the door
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> ExitBox;

	// Physical door blocker collider. Blocks passage when active. Automatically disabled during valid access.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxBlock;

	// Tag that the key must have to open this door
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTag KeyAccessTag;

	// Characters who have one of these tags can pass freely through the door
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTagContainer CharacterAccessTags;

	// Whether the character need to use a key to enter this door
	UPROPERTY(EditAnywhere, Category="Access", Replicated, SaveGame)
	bool bEnterRequiresKey = false;

	// Whether the character need to use a key to exit this door
	UPROPERTY(EditAnywhere, Category="Access", Replicated, SaveGame)
	bool bExitRequiresKey = false;

	UFUNCTION()
	void OnEnterBoxOverlapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitBoxOverlapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDoorwayOverlapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Sets whether a character can pass through a door
	void SetLockDoorway(const AEscapeChroniclesCharacter* Character, bool IsLock) const;

	// Removes 1 unit of durability
	void UseKey(const AEscapeChroniclesCharacter* Character) const;

	// Characters that can potentially enter through the door
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> EnteringCharactersPool;

	// Characters that can potentially exit through the door
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> ExitingCharactersPool;

	// Characters who are in the process of going through the door
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> ConfirmedCharactersPool;
};
