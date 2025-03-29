// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InteractPopupWidget.h"

void UInteractPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
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
