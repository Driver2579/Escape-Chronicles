// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystem/AttributeSets/CombatAttributeSet.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "GameFramework/Actor.h"
#include "ActivitySpot.generated.h"

struct FStreamableHandle;
class AEscapeChroniclesCharacter;
class UInteractableComponent;
class UGameplayEffect;

UCLASS()
class ESCAPECHRONICLES_API AActivitySpot : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AActivitySpot();

	bool SetCharacterOccupyingSpot(AEscapeChroniclesCharacter* Character);
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere)
	int32 EffectLevel;

	FActiveGameplayEffectHandle ActiveEffectSpecHandle;
	
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UAnimMontage>> AnimMontages;

	int32 SelectedAnimMontage;

	UPROPERTY(EditAnywhere)
	FName AttachSocketName;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInteractableComponent> InteractableComponent;
	

	
	void InteractHandler(UInteractionManagerComponent* InteractionManagerComponent);

	UPROPERTY(ReplicatedUsing="OnRep_CharacterOccupyingSpot")
	TObjectPtr<AEscapeChroniclesCharacter> CharacterOccupyingSpot;

	UFUNCTION()
	void OnRep_CharacterOccupyingSpot(AEscapeChroniclesCharacter* OldValue);

	

	void OccupySpot(AEscapeChroniclesCharacter* Character);
	void UnoccupySpot(AEscapeChroniclesCharacter* Character);
	
	FName CachedCapsuleCollisionProfileName;
	FTransform CachedMeshTransform;
	USceneComponent* CachedMeshAttachParent;


	
	void UnoccupyIfAttributeHasDecreased(const FOnAttributeChangeData& AttributeChangeData);
	
	FDelegateHandle UnoccupyIfAttributeHasDecreasedDelegateHandle;


	
	
	void OnAnimMontageLoaded();
	void OnGameplayEffectLoaded();

	TSharedPtr<FStreamableHandle> AnimMontageHandle;
	TSharedPtr<FStreamableHandle> GameplayEffectHandle;

	void CancelAnimationAndEffect(AEscapeChroniclesCharacter* Character);
};
