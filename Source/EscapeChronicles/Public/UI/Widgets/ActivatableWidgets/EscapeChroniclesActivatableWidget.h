// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonActivatableWidget.h"
#include "HUDs/EscapeChroniclesHUD.h"
#include "UI/Common/Enums/WidgetInputMode.h"
#include "EscapeChroniclesActivatableWidget.generated.h"

class AEscapeChroniclesHUD;

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	AEscapeChroniclesHUD* GetOwningHUD() const;

	template <typename T = UEscapeChroniclesActivatableWidget>
	T* OpenWidget(TSubclassOf<UEscapeChroniclesActivatableWidget> WidgetClass)
	{
		AEscapeChroniclesHUD* HUD = GetOwningHUD();

		if (ensureAlways(IsValid(HUD)))
		{
			return HUD->AddWidget<T>(WidgetClass);
		}
		
		return nullptr;
	}
};
