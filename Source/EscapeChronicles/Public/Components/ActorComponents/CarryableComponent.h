// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "CarryableComponent.generated.h"

UCLASS(BlueprintType, Blueprintable,meta = (BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UCarryableComponent : public UInteractableComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Carring")
	FName SocketToAttachWhenIsCarrying;
	
	UPROPERTY(EditDefaultsOnly, Category="Carring")
	FTransform TransformWhenIsCarrying;

	void Carry(UInteractionManagerComponent* InteractionManagerComponent);

	FTransform DefaultTransform;
};
