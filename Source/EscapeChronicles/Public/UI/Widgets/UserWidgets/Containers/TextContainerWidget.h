// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "TextContainerWidget.generated.h"

class UCommonTextBlock;

// A widget that simply contains a text block
UCLASS()
class ESCAPECHRONICLES_API UTextContainerWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UCommonTextBlock* GetTextBlock() const { return TextBlock; }

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock;
};