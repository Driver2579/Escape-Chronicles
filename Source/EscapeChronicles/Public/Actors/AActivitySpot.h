// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "GameFramework/Actor.h"
#include "AActivitySpot.generated.h"

struct FStreamableHandle;
class AEscapeChroniclesCharacter;
class UInteractableComponent;
class UGameplayEffect;

UCLASS()
class ESCAPECHRONICLES_API AAActivitySpot : public AActor
{
	GENERATED_BODY()
	
public:
	AAActivitySpot();
	
	void OccupySpot(AEscapeChroniclesCharacter* Character);
	void VacateSeat();
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UAnimMontage> AnimMontage;

	UPROPERTY(EditAnywhere)
	FName AttachSocketName;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInteractableComponent> InteractableComponent;

	void OnGameplayEffectLoaded();
	void OnAnimMontageLoaded();

	TSharedPtr<FStreamableHandle> GameplayEffectHandle;
	TSharedPtr<FStreamableHandle> AnimMontageHandle;

	void InteractHandler(UInteractionManagerComponent* InteractionManagerComponent);

	TWeakObjectPtr<AEscapeChroniclesCharacter> OccupyingSpotCharacter;
};
