// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDs/EscapeChroniclesHUD.h"

#include "UI/Widgets/ActivatableWidgets/EscapeChroniclesContainerWidget.h"
#include "UI/Widgets/ActivatableWidgets/MenuWidgets/MenuWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

AEscapeChroniclesHUD::AEscapeChroniclesHUD()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEscapeChroniclesHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (ensureAlways(IsValid(ContainerWidgetClass)))
	{
		ContainerWidget = CreateWidget<UEscapeChroniclesContainerWidget>(GetOwningPlayerController(),
				ContainerWidgetClass.Get());
	}
	
	if (!ensureAlways(IsValid(ContainerWidget)))
	{
		return;
	}

	ContainerWidget->AddToViewport();

	CurrentMenuTag = RootMenuTag;
}

void AEscapeChroniclesHUD::GoToMenu(FGameplayTag NewMenuTag)
{
	if (!ensureAlways(NewMenuTag.IsValid()) || NewMenuTag == CurrentMenuTag)
	{
		return;
	}

	if (NewMenuTag == RootMenuTag)
	{
		GoToRootMenu();
		
		return;
	}

	const TSubclassOf<UMenuWidget>* StateWidget = Menus.Find(NewMenuTag);

	if (!ensureAlways(StateWidget))
	{
		return;
	}
	
	UCommonActivatableWidgetStack* WidgetStack = ContainerWidget->GetWidgetStack();

	if (!ensureAlways(IsValid(WidgetStack)))
	{
		return;
	}

	WidgetStack->ClearWidgets();
	
	const UMenuWidget* MenuWidget = WidgetStack->AddWidget<UMenuWidget>(*StateWidget);

	if (!ensureAlways(IsValid(MenuWidget)))
	{
		return;
	}

	// Since when exiting this menu we will definitely get to the root widget, we must change CurrentMenuTag
	MenuWidget->OnDeactivated().AddLambda([&]()
	{
		CurrentMenuTag = RootMenuTag;
	});
	
	CurrentMenuTag = NewMenuTag;
}

void AEscapeChroniclesHUD::GoToRootMenu()
{
	UCommonActivatableWidgetStack* WidgetStack = ContainerWidget->GetWidgetStack();

	if (!ensureAlways(IsValid(WidgetStack)))
	{
		return;
	}

	WidgetStack->ClearWidgets();
	CurrentMenuTag = RootMenuTag;
}