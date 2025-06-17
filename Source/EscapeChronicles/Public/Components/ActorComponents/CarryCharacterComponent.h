// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "CarryCharacterComponent.generated.h"

struct FStreamableHandle;
class AEscapeChroniclesCharacter;

/**
 * Component that allows a character to pick up and carry other characters.
 * Handles network synchronization, animations, and attachment physics.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPECHRONICLES_API UCarryCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCarryCharacterComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Sets the carried character (called on the server)
	bool SetCarriedCharacter(AEscapeChroniclesCharacter* InCarriedCharacter);

	AEscapeChroniclesCharacter* GetCarriedCharacter() const { return CarriedCharacter.Get(); }

private:
	// The currently carried character 
	UPROPERTY(Transient, ReplicatedUsing="OnRep_CarriedCharacter")
	TObjectPtr<AEscapeChroniclesCharacter> CarriedCharacter;

	UFUNCTION()
	void OnRep_CarriedCharacter(AEscapeChroniclesCharacter* OldCharacter)
	{
		ReplaceCarriedCharacter(OldCharacter, CarriedCharacter);
	}

	/**
	 * Replaces the old carried character with a new one (handles pickup/drop logic).
	 * @return False if new character was set but wasn't picked up successfully, true otherwise.
	 */
	bool ReplaceCarriedCharacter(AEscapeChroniclesCharacter* OldCarriedCharacter,
		AEscapeChroniclesCharacter* NewCarriedCharacter);

	// Picks up a character (attaches, loads animations)
	bool PickupCharacter(AEscapeChroniclesCharacter* InCarriedCharacter);

	// Drops a character (detaches, enables physics)
	void DropCharacter(AEscapeChroniclesCharacter* InCarriedCharacter) const;

	// Tags defining which characters can be carried
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer CarryableCharacterTags;

	// === Attaching ===

	// Socket name for attachment
	UPROPERTY(EditDefaultsOnly)
	FName AttachSocketName;

	// Relative transform for attachment
	UPROPERTY(EditDefaultsOnly)
	FTransform AttachSocketTransform;

	// === Animations ===

	// Animation for the carrying character
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> CarryingCharacterAnimMontage;

	// Animation for the carried character
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> CarriedCharacterAnimMontage;

	// Cached mesh of the carried character (for animation)
	TWeakObjectPtr<USkeletalMeshComponent> CachedCarriedCharacterMesh;

	void OnCarryAnimMontagesLoaded();

	TSharedPtr<FStreamableHandle> CarryingCharacterAnimMontageHandle;
	TSharedPtr<FStreamableHandle> CarriedCharacterAnimMontageHandle;

	TSharedPtr<FStreamableHandle> CarryAnimMontagesLoadedHandle;
};
