// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/PromptWidget.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"
#include "InformPopup.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UInformPopup : public UPromptWidget
{
	GENERATED_BODY()
	
public:
	DECLARE_MULTICAST_DELEGATE(FOnCloseDelegate)

	FOnCloseDelegate OnClose;

	void SetDisplayedText(const FText& InTitleText) const
	{
		if (ensureAlways(DisplayedText))
		{
			DisplayedText->SetText(InTitleText);
		}
	}

protected:
	virtual void NativeOnInitialized() override
	{
		Super::NativeOnInitialized();
	
		if (ensureAlways(CloseButton))
		{
			CloseButton->OnClicked().AddUObject(this, &ThisClass::OnCloseButtonClicked);
		}
	}

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DisplayedText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> CloseButton;

	UFUNCTION()
	void OnCloseButtonClicked()
	{
		OnClose.Broadcast();

		DeactivateWidget();
	}
};
