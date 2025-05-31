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

// Actor is a place with which the character can interact and receive an effect
UCLASS()
class ESCAPECHRONICLES_API AActivitySpot : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOccupyingCharacterChanged, AEscapeChroniclesCharacter* Character);
	
	AActivitySpot();

	/**
	 * Determines who is currently occupying
	 * @param Character The one who wants to occupy, or nullptr to unoccupy current character
	 * @return True on success, false otherwise
	 */
	bool SetOccupyingCharacter(AEscapeChroniclesCharacter* Character);

	AEscapeChroniclesCharacter* GetOccupyingCharacter()
	{
		return CachedOccupyingCharacter;
	}
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	void AddOccupyingCharacterChangedHandler(const FOnOccupyingCharacterChanged::FDelegate& Callback);
	
protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OccupySpot(AEscapeChroniclesCharacter* Character);
	virtual void UnoccupySpot(AEscapeChroniclesCharacter* Character);
	
private:
	FOnOccupyingCharacterChanged OnOccupyingCharacterChanged;
	
	UPROPERTY(EditAnywhere)
	FName AttachSocketName;
    
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	// === Interaction ===
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInteractableComponent> InteractableComponent;
    
	void InteractHandler(UInteractionManagerComponent* InteractionManagerComponent);
    
	// === Effect ===
    
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere)
	int32 EffectLevel;

	FActiveGameplayEffectHandle ActiveEffectSpecHandle;

	// === Animation montage ===
    
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UAnimMontage>> AnimMontages;

	int32 SelectedAnimMontage;
	
	// === Occupying ===
	UPROPERTY(ReplicatedUsing="OnRep_CachedOccupyingCharacter")
	TObjectPtr<AEscapeChroniclesCharacter> CachedOccupyingCharacter;

	UFUNCTION()
	void OnRep_CachedOccupyingCharacter(AEscapeChroniclesCharacter* OldValue);

	FName CachedCapsuleCollisionProfileName;
	FTransform CachedMeshTransform;
	TWeakObjectPtr<USceneComponent> CachedMeshAttachParent;

	// === Unoccupy handlers ===

	void OnOccupyingCharacterHealthChanged(const FOnAttributeChangeData& AttributeChangeData);

	FDelegateHandle UnoccupyIfAttributeHasDecreasedDelegateHandle;

	// === Asset loading ===

	void OnAnimMontageLoaded();
	void OnGameplayEffectLoaded();

	TSharedPtr<FStreamableHandle> AnimMontageHandle;
	TSharedPtr<FStreamableHandle> GameplayEffectHandle;

	void CancelAnimationAndEffect(AEscapeChroniclesCharacter* Character);
};
