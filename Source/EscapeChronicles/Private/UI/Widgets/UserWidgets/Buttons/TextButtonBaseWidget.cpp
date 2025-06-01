// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"

void UTextButtonBaseWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	
	TSharedPtr<SWidget> SlateWidgetDetectingDrag = this->GetCachedWidget();
	if ( SlateWidgetDetectingDrag.IsValid() )
	{
		Reply.NativeReply = Reply.NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), EKeys::LeftMouseButton);
	}
	
	if (DisplayedText)
	{
		DisplayedText->SetText(ButtonText);
	}
}
