// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/PromptWidget.h"
#include "ConfirmationPopup.generated.h"

class UTextButtonBaseWidget;

// Standard confirmation dialog widget for user decision flows.
UCLASS()
class ESCAPECHRONICLES_API UConfirmationPopup : public UPromptWidget
{
	GENERATED_BODY()
	
public:
	// Delegate to receive a response from the user 
	DECLARE_MULTICAST_DELEGATE_OneParam(FResultDelegate, bool bConfirmed)
	
	FResultDelegate& OnResult() { return ResultDelegate; }
	
	void SetDisplayedText(const FText& InTitleText) const;
	
protected:
	virtual void NativeOnInitialized() override;
	
private:
	FResultDelegate ResultDelegate;

	// Displayed text when the widget is displayed
	UPROPERTY(meta=(BindWidget))  
	TObjectPtr<UTextBlock> DisplayedText;

	// Confirmation action button 
	UPROPERTY(meta=(BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> ConfirmButton;

	// Cancel action button 
	UPROPERTY(meta=(BindWidget))  
	TObjectPtr<UTextButtonBaseWidget> CancelButton;
	
	UFUNCTION()  
	void OnConfirmButtonClicked();

	UFUNCTION()  
	void OnCancelButtonClicked();
};
