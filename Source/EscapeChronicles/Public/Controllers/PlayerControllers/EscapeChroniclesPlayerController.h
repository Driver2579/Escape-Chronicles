// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "EscapeChroniclesPlayerController.generated.h"

class UInputConfig;

UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesPlayerController : public APlayerController, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override final;
	class UEscapeChroniclesAbilitySystemComponent* GetEscapeChroniclesAbilitySystemComponent() const;

	// Binds all input actions in the given input config
	void BindInputConfig(UEnhancedInputComponent* EnhancedInputComponent, const UInputConfig* InputConfig);

protected:
	virtual void SetupInputComponent() override;

private:
	// Input configs to bind 
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TArray<TObjectPtr<UInputConfig>> InputConfigs;

	void InputTriggered(const FInputActionValue& InputActionValue, FGameplayTag InputTag);
	void InputCompleted(FGameplayTag InputTag);
};