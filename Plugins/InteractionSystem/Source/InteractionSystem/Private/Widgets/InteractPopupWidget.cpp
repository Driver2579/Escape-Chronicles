// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/InteractPopupWidget.h"

void UInteractPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initially show the first frame
	PlayAnimationForward(PopupAnimation);
	PauseAnimation(PopupAnimation);
}

void UInteractPopupWidget::ShowPopup()
{
	PlayAnimationForward(PopupAnimation);
}

void UInteractPopupWidget::HidePopup()
{
	PlayAnimationReverse(PopupAnimation);
}
