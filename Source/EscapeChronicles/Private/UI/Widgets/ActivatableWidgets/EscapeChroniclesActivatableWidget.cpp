// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/ActivatableWidgets/EscapeChroniclesActivatableWidget.h"

#include "HUDs/EscapeChroniclesHUD.h"

AEscapeChroniclesHUD* UEscapeChroniclesActivatableWidget::GetOwningHUD() const
{
	const APlayerController* PlayerController = GetOwningPlayer();

	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	
	return PlayerController->GetHUD<AEscapeChroniclesHUD>();
}
