// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "TextButtonBaseWidget.generated.h"

// Base class for text-based buttons in Common UI system
UCLASS()
class ESCAPECHRONICLES_API UTextButtonBaseWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	const FText& GetText() { return ButtonText; }

protected:
	virtual void NativePreConstruct() override;

private:
	// The text displayed on the button
	UPROPERTY(EditAnywhere, Category="Content")  
	FText ButtonText;

	// Bound text block component that renders ButtonText
	UPROPERTY(Transient, meta=(BindWidget))  
	TObjectPtr<class UTextBlock> DisplayedText;
};