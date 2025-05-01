// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/TextButtonBaseWidget.h"

#include "Components/TextBlock.h"

void UTextButtonBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ensureAlways(DisplayedText))
	{
		DisplayedText->SetText(ButtonText);
	}
}
