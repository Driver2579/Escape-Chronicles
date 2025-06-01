// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDs/EscapeChroniclesHUD.h"

#include "UI/Widgets/UserWidgets/RootContainerWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

AEscapeChroniclesHUD::AEscapeChroniclesHUD()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEscapeChroniclesHUD::BeginPlay()
{
	Super::BeginPlay();

	// === Create and add RootWidget to viewport ===
	
	APlayerController* PlayerController = GetOwningPlayerController();
	
	if (!ensureAlways(IsValid(RootWidgetClass)) || !ensureAlways(IsValid(PlayerController)))
	{
		return;
	}
	
	RootWidget = CreateWidget<URootContainerWidget>(PlayerController, RootWidgetClass);
	
	if (!ensureAlways(RootWidget))
	{
		return;
	}

	RootWidget->AddToViewport();
	
	// === Configure root content handling ===

	const UCommonActivatableWidgetStack* ContentStack = RootWidget->GetContentStack();

	if (!ensureAlways(IsValid(ContentStack)))
	{
		return;
	}

	const UCommonActivatableWidget* RootContent = ContentStack->GetRootContent();

	if (!ensureAlways(IsValid(RootContent)))
	{
		return;
	}

	RootContent->OnActivated().AddWeakLambda(this, [this]
	{
		SetInputMode(RootInputMode, bRootCursorVisible);
	});
}

void AEscapeChroniclesHUD::GoTo(const FGameplayTag& RouteName)
{
	if (!ensureAlways(RouteName.IsValid()) || !ensureAlways(RootWidget))
	{
		return;
	}

	UCommonActivatableWidgetStack* ContentStack = RootWidget->GetContentStack();
	
	const FHUDRoute& Route = *Routes.Find(RouteName);

	if (!ensureAlways(IsValid(Route.WidgetClass)) || !ensureAlways(IsValid(ContentStack)))
	{
		return;
	}
	
	ContentStack->ClearWidgets();
	ContentStack->AddWidget(Route.WidgetClass);

	SetInputMode(Route.InputMode, Route.bCursorVisible);
}

void AEscapeChroniclesHUD::GoToRoot() const
{
	if (!RootWidget)
	{
		return;
	}

	UCommonActivatableWidgetStack* ContentStack = RootWidget->GetContentStack();
	
	if (ensureAlways(IsValid(ContentStack)))
	{
		ContentStack->ClearWidgets();
	}
}

void AEscapeChroniclesHUD::SetInputMode(const ERouteInputMode NewInputMode, const bool bNewCursorVisible) const
{
	APlayerController* PlayerController = GetOwningPlayerController();

	if (!ensureAlways(IsValid(PlayerController)))
	{
		return;
	}

	PlayerController->SetShowMouseCursor(bNewCursorVisible);

	switch (NewInputMode)
	{
		case ERouteInputMode::Game:
			PlayerController->SetInputMode(FInputModeGameOnly());
			break;
		
		case ERouteInputMode::UI:
			PlayerController->SetInputMode(FInputModeUIOnly());
			break;
		
		case ERouteInputMode::GameAndUI:
			PlayerController->SetInputMode(FInputModeGameAndUI());
			break;
	}
}
