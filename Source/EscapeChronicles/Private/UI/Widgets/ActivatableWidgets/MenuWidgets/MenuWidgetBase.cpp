// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/UI/Widgets/ActivatableWidgets/MenuWidgets/MenuWidgetBase.h"

#include "HUDs/EscapeChroniclesHUD.h"

UMenuWidgetBase::UMenuWidgetBase()
{
}

bool UMenuWidgetBase::NativeOnHandleBackAction()
{
	if (!StateTagOnBackAction.IsValid())
	{
		return Super::NativeOnHandleBackAction();
	}

	const APlayerController* PlayerController = GetOwningPlayer();

	if (!ensureAlways(IsValid(PlayerController)))
	{
		return false;
	}
	
	AEscapeChroniclesHUD* HUD = PlayerController->GetHUD<AEscapeChroniclesHUD>();
	
	if (!ensureAlways(IsValid(HUD)))
	{
		return false;
	}

	HUD->ChangeState(StateTagOnBackAction);
	
	return true;
}
