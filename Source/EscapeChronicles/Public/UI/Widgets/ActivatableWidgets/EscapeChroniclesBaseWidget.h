// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonActivatableWidget.h"
#include "EscapeChroniclesBaseWidget.generated.h"

class UCommonActivatableWidgetStack;

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesBaseWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UCommonActivatableWidgetStack* GetWidgetStack() const { return WidgetStack; }
	
private:
	UPROPERTY(meta = (BindWidget))
	UCommonActivatableWidgetStack* WidgetStack;
};
