﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "RootContainerWidget.generated.h"

class AEscapeChroniclesHUD;

/**
 * Root UI container widget that manages two separate widget stacks:
 * - ContentStack: Primary layer for game UI (menus, HUD elements)
 * - PromptStack: Overlay layer for transient UI (popups, dialogs, tooltips)
 */
UCLASS()
class ESCAPECHRONICLES_API URootContainerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UCommonActivatableWidgetStack* GetContentStack() const { return ContentStack; }
	UCommonActivatableWidgetStack* GetPromptStack() const { return PromptStack; }
	
private:
	// Stack managing persistent UI elements (menus, HUD)
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> ContentStack;

	// Stack managing temporary UI elements (popups, notifications)
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PromptStack;
};