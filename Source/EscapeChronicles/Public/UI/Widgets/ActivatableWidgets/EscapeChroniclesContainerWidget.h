// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonActivatableWidget.h"
#include "EscapeChroniclesContainerWidget.generated.h"

class UCommonActivatableWidgetStack;

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesContainerWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UCommonActivatableWidgetStack* GetWidgetStack() const { return WidgetStack; }
	
private:
	// TODO: Add a separate stack for backgrounds
	
	UPROPERTY(meta = (BindWidget))
	UCommonActivatableWidgetStack* WidgetStack;
};
