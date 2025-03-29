// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPopupWidget.generated.h"

/**
 * Can be used in InteractableComponent with a component to show a tooltip
 */
UCLASS()
class INTERACTIONSYSTEM_API UInteractPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Popup Show Events")
	void OnPopupShow();

	UFUNCTION(BlueprintImplementableEvent, Category = "Popup Hide Events")
	void OnPopupHide();
};
