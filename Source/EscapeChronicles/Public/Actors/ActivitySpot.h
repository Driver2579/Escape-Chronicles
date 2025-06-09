// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ActiveGameplayEffectHandle.h"
#include "AbilitySystem/AttributeSets/CombatAttributeSet.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "GameFramework/Actor.h"
#include "ActivitySpot.generated.h"

class UPlayerOwnershipComponent;
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
	AActivitySpot();

	int32 GetEffectLevel() const { return EffectLevel; }

	/**
	 * Determines who is currently occupying
	 * @param Character The one who wants to occupy, or nullptr to unoccupy current character
	 * @return True on success, false otherwise
	 */
	bool SetOccupyingCharacter(AEscapeChroniclesCharacter* Character);

	void SetEffectLevel(int32 InEffectLevel) { EffectLevel = InEffectLevel; }

	AEscapeChroniclesCharacter* GetOccupyingCharacter() const { return CachedOccupyingCharacter; }

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnOccupyingCharacterChanged, AEscapeChroniclesCharacter* Character);

	// Called when a character occupies or unoccupies this actor this delegate
	FOnOccupyingCharacterChanged OnOccupyingCharacterChanged;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Makes character occupy this actor (The value of CachedOccupyingCharacter does not change)
	virtual void OccupySpot(AEscapeChroniclesCharacter* Character);

	// Makes character unoccupy this actor (The value of CachedOccupyingCharacter does not change)
	virtual void UnoccupySpot(AEscapeChroniclesCharacter* Character);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPlayerOwnershipComponent> PlayerOwnershipComponent;

	// === Attaching ===

	// Mesh for display, interaction and attach to when character occupying
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Movement", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Socket on the mesh that the character will attach to when it is occupying
	UPROPERTY(EditAnywhere)
	FName AttachSocketName;

	FTransform CachedMeshTransform;
	TWeakObjectPtr<USceneComponent> CachedMeshAttachParent;

	// === Interaction ===

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Movement", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteractableComponent> InteractableComponent;

	void OnInteract(UInteractionManagerComponent* InteractionManagerComponent);

	// === Effect ===

	// Effect that is applied on the occupied character and is removed when he gets up
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UGameplayEffect> OccupingEffectClass;

	UPROPERTY(EditAnywhere)
	int32 EffectLevel;

	FActiveGameplayEffectHandle OccupingEffectSpecHandle;

	// === Animation montage ===

	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UAnimMontage>> OccupingAnimMontages;

	int32 SelectedOccupingAnimMontage;

	// === Occupying ===

	UPROPERTY(ReplicatedUsing="OnRep_CachedOccupyingCharacter")
	TObjectPtr<AEscapeChroniclesCharacter> CachedOccupyingCharacter;

	UFUNCTION()
	void OnRep_CachedOccupyingCharacter(AEscapeChroniclesCharacter* OldValue);

	// OccupySpot does not work on characters with these tags
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer OccupyingBlockedTags;

	// === Events when unoccupy occupied character ===

	// When health decreases
	void OnOccupyingCharacterHealthChanged(const FOnAttributeChangeData& AttributeChangeData);
	FDelegateHandle UnoccupyIfAttributeHasDecreasedDelegateHandle;

	// === Asset loading ===

	void OnOccupyingAnimMontageLoaded();
	void OnOccupyingEffectLoaded();

	TSharedPtr<FStreamableHandle> OccupyingAnimMontageHandle;
	TSharedPtr<FStreamableHandle> OccupyingEffectHandle;

	void CancelOccupyingAnimationAndEffect(AEscapeChroniclesCharacter* Character);
};
