// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Common/Delegates/PlayerStateDelegates.h"
#include "EscapeChroniclesPlayerController.generated.h"

class AEscapeChroniclesPlayerState;
class UEscapeChroniclesAbilitySystemComponent;
class AEscapeChroniclesCharacter;
class UInputConfig;

struct FInputActionValue;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesPlayerController : public APlayerController, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEscapeChroniclesPlayerController();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override final;
	UEscapeChroniclesAbilitySystemComponent* GetEscapeChroniclesAbilitySystemComponent() const;

	AEscapeChroniclesCharacter* GetEscapeChroniclesCharacter() const;

	virtual void InitPlayerState() override;
	virtual void OnRep_PlayerState() override;

	void CallOrRegister_OnPlayerStateInitialized(const FOnPlayerStateInitializedDelegate::FDelegate& Callback);

	// Adds all input mapping contexts and binds all input actions in the given input config
	void BindInputConfig(UEnhancedInputComponent* EnhancedInputComponent, const UInputConfig* InputConfig);

	virtual void OnUnPossess() override;

protected:
	virtual void SetupInputComponent() override;

	/**
	 * Binds all input configs set up in the InputConfigs array.
	 * @remark Should be called only when both InputComponent and PlayerState are valid.
	 */
	virtual void BindInputConfigs();

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AEscapeChroniclesPlayerState> PlayerStateClassOverride;

	FOnPlayerStateInitializedDelegate OnPlayerStateInitialized;

	bool bBindInputConfigsOnPlayerStateInitialized = false;

	// Input configs to bind at the start of the game
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TArray<TObjectPtr<UInputConfig>> InputConfigs;

	void AddMappingContexts(const UInputConfig* InputConfig) const;

	void BindNativeInputActions(UEnhancedInputComponent* EnhancedInputComponent, const UInputConfig* InputConfig);
	void BindAbilityInputActions(UEnhancedInputComponent* EnhancedInputComponent, const UInputConfig* InputConfig);

	void AbilityInputStarted(FGameplayTag InputTag);
	void AbilityInputCompleted(FGameplayTag InputTag);

	// Calls Move function on the character
	void MoveActionTriggered(const FInputActionValue& Value);

	// Calls StopMoving function on the character
	void MoveActionCompleted();

	// Calls Look function on the character
	void LookActionTriggered(const FInputActionValue& Value);
};