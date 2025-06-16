// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Scanner.generated.h"

class AEscapeChroniclesCharacter;
class UGameplayEffect;
class UBoxComponent;

struct FStreamableHandle;

/**
 * A scanner is an actor that has a trigger which applies the gameplay effect to the character when he enters it with
 * any contraband items in his inventory unless he has an item with a contraband bag fragment or unless he has any of
 * the specified immunity tags.
 */
UCLASS()
class ESCAPECHRONICLES_API AScanner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AScanner();

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> TriggerComponent;

	/**
	 * If the character has any of these tags, he will not be affected by the scanner. If this container is empty, all
	 * characters will be affected.
	 */
	UPROPERTY(EditAnywhere, Category="Gameplay Tags")
	FGameplayTagContainer ImmunityTags;

	// The gameplay effect to apply when the player enters the scanner with contraband items
	UPROPERTY(EditAnywhere, Category="Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> GameplayEffectClass;

	// Applies a gameplay effect to the given character
	void OnGameplayEffectClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
		const AEscapeChroniclesCharacter* Character) const;
};