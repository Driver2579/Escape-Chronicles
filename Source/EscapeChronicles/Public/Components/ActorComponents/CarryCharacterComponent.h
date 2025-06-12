// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "CarryCharacterComponent.generated.h"

struct FStreamableHandle;
class AEscapeChroniclesCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPECHRONICLES_API UCarryCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCarryCharacterComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetCarriedCharacter(AEscapeChroniclesCharacter* InCarriedCharacter);
	AEscapeChroniclesCharacter* GetCarriedCharacter() const { return CarriedCharacter.Get(); }

private:
	UPROPERTY(Transient, ReplicatedUsing="OnRep_CarriedCharacter")
	TObjectPtr<AEscapeChroniclesCharacter> CarriedCharacter;

	UFUNCTION()
	void OnRep_CarriedCharacter(AEscapeChroniclesCharacter* OldCharacter)
	{
		ReplaceCarriedCharacter(OldCharacter, CarriedCharacter);
	}

	void ReplaceCarriedCharacter(const AEscapeChroniclesCharacter* OldCarriedCharacter,
		const AEscapeChroniclesCharacter* NewCarriedCharacter);

	void PickupCharacter(USkeletalMeshComponent* CarriedCharacterMesh);
	void DropCharacter(USkeletalMeshComponent* CarriedCharacterMesh) const;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer CarryableCharacterTags;

	// === Attaching ===

	UPROPERTY(EditDefaultsOnly)
	FName AttachSocketName;

	UPROPERTY(EditDefaultsOnly)
	FTransform AttachSocketTransform;

	// === Animations ===

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> CarryingCharacterAnimMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> CarriedCharacterAnimMontage;

	TWeakObjectPtr<USkeletalMeshComponent> CachedCarriedCharacterMesh;

	void OnCarryAnimMontagesLoaded();

	TSharedPtr<FStreamableHandle> CarryingCharacterAnimMontageHandle;
	TSharedPtr<FStreamableHandle> CarriedCharacterAnimMontageHandle;

	TSharedPtr<FStreamableHandle> CarryAnimMontagesLoadedHandle;
};
