// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "UI/Widgets/UserWidgets/RootContainerWidget.h"
#include "CommonUIUtils.h"
#include "PromptWidget.h"
#include "RoutableWidget.generated.h"

enum class ERoutableInputMode : uint8;

// Extends Common UI's activatable widgets with routing-specific functionality to work with URootContainerWidget
UCLASS()
class ESCAPECHRONICLES_API URoutableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	// Gets the root container widget that owns this widget
	URootContainerWidget* GetOwningRootWidget() const
	{
		return CommonUIUtils::GetOwningUserWidget<URootContainerWidget>(this);
	}
	
	/**
	 * Pushes a new widget onto the main content stack
	 * @param WidgetClass Class of widget to spawn
	 * @return Pointer to created widget or nullptr on failure
	 */
	template <typename T = URoutableWidget>
	T* PushWidget(const TSubclassOf<URoutableWidget> WidgetClass)
	{
		const URootContainerWidget* RootWidget = GetOwningRootWidget();
		
		if (!ensureAlways(IsValid(RootWidget)))
		{
			return nullptr;
		}

		UCommonActivatableWidgetStack* ContentStack = RootWidget->GetContentStack();

		if (!ensureAlways(IsValid(ContentStack)))
		{
			return nullptr;
		}
	
		return ContentStack->AddWidget<T>(WidgetClass);
	}

	/**
	 * Pushes a transient widget onto the prompt stack
	 * @param WidgetClass Class of widget to spawn
	 * @return Pointer to created widget or nullptr on failure
	 */
	template <typename T = UPromptWidget>
	T* PushPrompt(const TSubclassOf<UPromptWidget> WidgetClass)
	{
		const URootContainerWidget* RootWidget = GetOwningRootWidget();
		
		if (!ensureAlways(IsValid(RootWidget)))
		{
			return nullptr;
		}

		UCommonActivatableWidgetStack* PromptStack = RootWidget->GetPromptStack();

		if (!ensureAlways(IsValid(PromptStack)))
		{
			return nullptr;
		}
	
		return PromptStack->AddWidget<T>(WidgetClass);
	}
};
