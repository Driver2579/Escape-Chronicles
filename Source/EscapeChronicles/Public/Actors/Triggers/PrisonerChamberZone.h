// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/Saveable.h"
#include "PrisonerChamberZone.generated.h"

class AActivitySpot;
class AEscapeChroniclesCharacter;
class UBoxComponent;
class UPlayerOwnershipComponent;

UCLASS()
class ESCAPECHRONICLES_API APrisonerChamberZone : public AActor, public ISaveable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APrisonerChamberZone();

	UBoxComponent* GetOuterZoneBoxComponent() const { return OuterZoneBoxComponent; }
	UBoxComponent* GetInnerZoneBoxComponent() const { return InnerZoneBoxComponent; }

	UPlayerOwnershipComponent* GetPlayerOwnershipComponent() const { return PlayerOwnershipComponent; }

	/**
	 * @return True if the character is currently in the chamber, meaning he has overlapped the inner zone and didn't
	 * leave the outer zone yet.
	 * @remark This doesn't check if the character is in his own chamber, it just checks if he is in this particular
	 * chamber.
	 */
	bool IsCharacterInChamber(const AEscapeChroniclesCharacter* Character) const;

	/**
	 * @param bOutHasOwningCharacter Whether the chamber has any owning character currently in the game.
	 * @return True if the owning character of this chamber is currently in it, meaning he has overlapped the inner zone
	 * and didn't leave the outer zone yet.
	 */
	bool IsOwningCharacterInThisChamber(bool& bOutHasOwningCharacter) const;

	/**
	 * Checks if there is a bed that overlaps this chamber and that has the same owner as this chamber, and if this bed
	 * is currently occupied (somebody is sleeping in it).
	 * @param bOutHasBedWithSameOwningCharacter Whether the chamber has any bed that overlaps it and has the same owning
	 * character. Always false if this chamber doesn't have any owning character.
	 * @return True if all described conditions are met.
	 * @return False if this chamber doesn't have any owning character, or if there is no bed that overlaps this
	 * chamber, or if the bed is currently empty (nobody is sleeping in it).
	 */
	bool IsOwningCharacterBedOccupied(bool& bOutHasBedWithSameOwningCharacter) const;

	/**
	 * Checks if the character is currently in his own chamber.
	 * @param Character Character to check.
	 * @param bOutHasOwningChamber Whether the character has any owning chamber.
	 * @return Whether the character is in his own chamber.
	 */
	static bool IsCharacterInHisChamber(const AEscapeChroniclesCharacter* Character,
		bool& bOutHasOwningChamber);

	// Gets all characters that own any chambers and aren't currently in any of them
	static void GetAllCharactersOutsideTheirChambers(const UObject* WorldContextObject,
		TArray<AEscapeChroniclesCharacter*>& OutCharactersOutsideTheirChambers);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int OtherBodyIndex);

private:
	// A zone the player needs to stop overlapping with to stop being considered he is in the chamber
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> OuterZoneBoxComponent;

	/**
	 * A zone the player needs to overlap with to start being considered he is in the chamber. This trigger should be
	 * fully inside the Outer Zone trigger.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> InnerZoneBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UPlayerOwnershipComponent> PlayerOwnershipComponent;

	// Characters that overlapped the inner zone and didn't leave the outer zone yet
	TSet<TWeakObjectPtr<const AEscapeChroniclesCharacter>> CharactersInChamber;

	// The class of the bed that the prisoner that owns this chamber should sleep in
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AActivitySpot> PrisonerBedClass;
};