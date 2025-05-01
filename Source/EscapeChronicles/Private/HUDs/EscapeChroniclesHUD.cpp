// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDs/EscapeChroniclesHUD.h"

#include "UI/Widgets/ActivatableWidgets/EscapeChroniclesBaseWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void AEscapeChroniclesHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (ensureAlways(IsValid(BaseWidgetClass)))
	{
		BaseWidget = CreateWidget<UEscapeChroniclesBaseWidget>(GetOwningPlayerController(),
				BaseWidgetClass.Get());
	}
	
	if (ensureAlways(IsValid(BaseWidget)))
	{
		BaseWidget->AddToViewport();
		ChangeState(DefaultState);
	}
}

void AEscapeChroniclesHUD::ChangeState(const FGameplayTag StateTag)
{
	if (StateTag == CurrentStateTag)
	{
		return;
	}
	
	const FHUDState* State = States.Find(StateTag);

	if (!ensureAlways(State && IsValid(State->WidgetClass)))
	{
		return;
	}

	// === Change current widget ===
	
	UCommonActivatableWidgetStack* WidgetStack = BaseWidget->GetWidgetStack();

	if (!ensureAlways(WidgetStack))
	{
		return;
	}

	WidgetStack->ClearWidgets();
	WidgetStack->AddWidget(State->WidgetClass);
	
	// === Change input mode ===
	
	FInputModeDataBase* CurrentInputMode;
	
	switch (State->InputMode)
	{
		case EInputMode::GameOnly: CurrentInputMode = new FInputModeGameOnly(); break;
		case EInputMode::UiOnly: CurrentInputMode = new FInputModeUIOnly(); break;
		case EInputMode::GameAndUi: CurrentInputMode = new FInputModeGameAndUI(); break;
		default: CurrentInputMode = new FInputModeGameOnly(); break;
	}

	APlayerController* PlayerController = GetOwningPlayerController();

	if (ensureAlways(IsValid(PlayerController)))
	{
		PlayerController->SetInputMode(*CurrentInputMode);
		PlayerController->SetShowMouseCursor(State->bShowMouseCursor);
	}
	
	// === Caching ===
	
	CurrentStateTag = StateTag;
}