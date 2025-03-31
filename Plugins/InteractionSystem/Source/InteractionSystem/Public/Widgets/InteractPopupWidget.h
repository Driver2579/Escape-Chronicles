// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractPopupWidget.generated.h"


/**
 * Can be used in InteractableComponent with a component to show a hint
 */
UCLASS()
class INTERACTIONSYSTEM_API UInteractPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowPopup();
	void HidePopup();

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(Transient, meta=(BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> PopupAnimation;
};
