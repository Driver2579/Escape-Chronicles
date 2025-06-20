// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/SettingsMenuWidget.h"

#include "CommonActivatableWidgetSwitcher.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/GraphicsSettingsMenuWidget.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget/SettingsMenus/SoundSettingsMenuWidget.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"

void USettingsMenuWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!SettingsWidgetsSwitcher)
	{
		return;
	}

	if (ensureAlways(IsValid(GraphicsSettingsMenuWidgetClass)))
	{
		GraphicsSettingsMenuWidget = CreateWidget<UGraphicsSettingsMenuWidget>(this,
			GraphicsSettingsMenuWidgetClass);

		SettingsWidgetsSwitcher->AddChild(GraphicsSettingsMenuWidget);
		SettingsWidgetsSwitcher->SetActiveWidget(GraphicsSettingsMenuWidget);
	}

	if (ensureAlways(IsValid(SoundSettingsMenuWidgetClass)))
	{
		SoundSettingsMenuWidget = CreateWidget<USoundSettingsMenuWidget>(this,
			SoundSettingsMenuWidgetClass);

		SettingsWidgetsSwitcher->AddChild(SoundSettingsMenuWidget);
	}
}

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (OpenGraphicsSettingsButton)
	{
		OpenGraphicsSettingsButton->OnClicked().AddUObject(this, &ThisClass::OnGraphicsSettingsMenuBackButtonClicked);
	}

	if (OpenSoundSettingsButton)
	{
		OpenSoundSettingsButton->OnClicked().AddUObject(this, &ThisClass::OnSoundSettingsMenuBackButtonClicked);
	}

	if (ApplyButton)
	{
		ApplyButton->OnClicked().AddUObject(this, &ThisClass::OnApplyButtonClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &ThisClass::OnBackButtonClicked);
	}
}

// ReSharper disable CppMemberFunctionMayBeConst
void USettingsMenuWidget::OnGraphicsSettingsMenuBackButtonClicked()
{
#if DO_CHECK
	check(SettingsWidgetsSwitcher);
#endif

	if (ensureAlways(GraphicsSettingsMenuWidget))
	{
		SettingsWidgetsSwitcher->SetActiveWidget(GraphicsSettingsMenuWidget);
	}
}

void USettingsMenuWidget::OnSoundSettingsMenuBackButtonClicked()
{
#if DO_CHECK
	check(SettingsWidgetsSwitcher);
#endif

	if (ensureAlways(SoundSettingsMenuWidget))
	{
		SettingsWidgetsSwitcher->SetActiveWidget(SoundSettingsMenuWidget);
	}
}
// ReSharper restore CppMemberFunctionMayBeConst

// ReSharper disable once CppMemberFunctionMayBeConst
void USettingsMenuWidget::OnApplyButtonClicked()
{
	OnApply.Broadcast();
}

void USettingsMenuWidget::OnBackButtonClicked()
{
	DeactivateWidget();
}