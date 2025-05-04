// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeChroniclesActivatableWidget.h"
#include "YesNoPopup.generated.h"

class UTextBlock;
class UTextButtonBaseWidget;

// A popup that gives you the choice to confirm or cancel
UCLASS()
class ESCAPECHRONICLES_API UYesNoPopup : public UEscapeChroniclesActivatableWidget
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(UYesNoPopup, FResultEvent, bool bConfirmed)
	FResultEvent& OnResult() { return ResultEvent; }
	
	void SetTitleText(const FText& InTitleText) { TitleText = InTitleText; }
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	
private:
	FResultEvent ResultEvent;
	
	UPROPERTY(EditAnywhere)  
	FText TitleText;
	
	UPROPERTY(meta = (BindWidget))  
	UTextBlock* DisplayedText;
	
	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* ConfirmButton;

	UPROPERTY(meta = (BindWidget))  
	UTextButtonBaseWidget* CancelButton;
	
	UFUNCTION()  
	void OnConfirmButtonClicked();

	UFUNCTION()  
	void OnCancelButtonClicked();
};