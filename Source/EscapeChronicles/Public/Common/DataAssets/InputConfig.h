// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

// Struct used to map an input action to a gameplay input tag and to store additional settings for the input action
USTRUCT(BlueprintType)
struct FAbilityInputActionSettings
{
	GENERATED_BODY()

	// Input tag to map the input action to
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;

	/**
	 * Whether the ability should be ended when the input action is completed. The ability will be activated on
	 * ETriggerEvent::Started anyway, but if this is true, it will also be ended on ETriggerEvent::Completed. If this
	 * is false, the ability should be ended by the ability itself.
	 */
	UPROPERTY(EditDefaultsOnly)
	bool bEndAbilityOnComplete = false;
};

// Non-mutable data asset that contains input configuration properties
UCLASS()
class ESCAPECHRONICLES_API UInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	const auto& GetInputMappingContexts() const { return InputMappingContexts; }

	const auto& GetNativeInputActions() const { return NativeInputActions; }
	const auto& GetAbilityInputActions() const { return AbilityInputActions; }

private:
	/**
	 * List of input mapping contexts used by the owner. These input mapping contexts should contain all input actions
	 * from NativeInputActions and AbilityInputActions.
	 */
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<const UInputMappingContext>> InputMappingContexts;

	/**
	 * List of input actions used by the owner. These input actions are mapped to a gameplay tag and must be manually
	 * bound.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TObjectPtr<const UInputAction>, FGameplayTag> NativeInputActions;

	/**
	 * List of input actions used by the owner. These input actions are mapped to a gameplay tag and are automatically
	 * bound to abilities with matching input tags.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<TObjectPtr<const UInputAction>, FAbilityInputActionSettings> AbilityInputActions;
};