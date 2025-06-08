// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

class AEscapeChroniclesCharacter;
class UBoxComponent;
class UPhysicsConstraintComponent;

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

	void SetEnterRequiresKey(const bool InEnterRequiresKey) { bEnterRequiresKey = InEnterRequiresKey; }
	void SetExitRequiresKey(const bool InExitRequiresKey) { bExitRequiresKey = InExitRequiresKey; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> EnterBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> ExitBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxOverlap;

	// Tag witch must have the key to open this door
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTag KeyAccessTag;

	// Characters who have one of these tags can pass freely through the door
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTagContainer CharacterAccessTags;
	
	UPROPERTY(EditAnywhere, Category="Access")
	bool bEnterRequiresKey = false;

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

	// Characters who are in the process of going through the door
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> ConfirmedCharactersPool;
};
