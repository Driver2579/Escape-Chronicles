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

protected:
	virtual void BeginPlay() override;

private:
	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> EnterBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> ExitBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxBlock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> DoorwayBoxOverlap;
	
	UPROPERTY(EditAnywhere, Category="Access")
	FGameplayTag KeyAccessTag;
	
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

	// Returns true if the key is needed in the opposite side position
	bool IsOppositeSideRequiresKey(const AEscapeChroniclesCharacter* Character) const;
	
	// Sets whether a character can pass through a door
	void SetLockDoorway(const AEscapeChroniclesCharacter* Character, bool IsLock) const;
	
	// Returns true if the character has access to this door
	bool HasCharacterAccess(const AEscapeChroniclesCharacter* Character) const;
	
	// Returns true if the character has a matching key
	bool HasCharacterMatchingKey(const AEscapeChroniclesCharacter* Character) const;
	
	// Removes 1 unit of durability if needed
	void UseKey(const AEscapeChroniclesCharacter* Character) const;

	// The characters who used the key
	TArray<TObjectPtr<AEscapeChroniclesCharacter>> ConfirmedCharactersPool;
};
