// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/ActivatableWidgets/MenuWidgets/MenuWidget.h"

void UMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetInputModeForCurrentWidget();
}

void UMenuWidget::SetInputModeForCurrentWidget() const
{
	FInputModeDataBase* CurrentInputMode;
	
	switch (InputMode)
	{
	case EWidgetInputMode::GameOnly: CurrentInputMode = new FInputModeGameOnly(); break;
	case EWidgetInputMode::UiOnly: CurrentInputMode = new FInputModeUIOnly(); break;
	case EWidgetInputMode::GameAndUi: CurrentInputMode = new FInputModeGameAndUI(); break;
	default: CurrentInputMode = new FInputModeGameOnly(); break;
	}

	APlayerController* PlayerController = GetOwningPlayer();

	if (ensureAlways(IsValid(PlayerController)))
	{
		PlayerController->SetInputMode(*CurrentInputMode);
		PlayerController->SetShowMouseCursor(bShowMouseCursor);
	}
}
