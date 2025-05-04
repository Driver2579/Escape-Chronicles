// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "TextButtonBaseWidget.generated.h"

// Base type for UI buttons that contain text
UCLASS()
class ESCAPECHRONICLES_API UTextButtonBaseWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	const FText& GetText() { return ButtonText; }
	
protected:
	virtual void NativePreConstruct() override;
	
private:
	UPROPERTY(EditAnywhere)  
	FText ButtonText;
	
	UPROPERTY(meta = (BindWidget))  
	class UTextBlock* DisplayedText;
};