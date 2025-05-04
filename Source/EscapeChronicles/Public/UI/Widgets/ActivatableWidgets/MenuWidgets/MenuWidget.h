// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UI/Common/Enums/WidgetInputMode.h"
#include "UI/Widgets/ActivatableWidgets/EscapeChroniclesActivatableWidget.h"
#include "MenuWidget.generated.h"

class AEscapeChroniclesHUD;

UCLASS()
class ESCAPECHRONICLES_API UMenuWidget : public UEscapeChroniclesActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnActivated() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	EWidgetInputMode InputMode;

	UPROPERTY(EditDefaultsOnly)
	bool bShowMouseCursor;
	
	void SetInputModeForCurrentWidget() const;
};