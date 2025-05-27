// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"

#include "Components/TextBlock.h"

void UTextButtonBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (DisplayedText)
	{
		DisplayedText->SetText(ButtonText);
	}
}
