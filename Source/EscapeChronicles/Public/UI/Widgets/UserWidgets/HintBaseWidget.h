// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommonUserWidget.h"
#include "Components/TextBlock.h"

#include "HintBaseWidget.generated.h"

class UTextBlock;
struct FInventorySlot;
class UItemSlotWidgetData;

UCLASS()
class ESCAPECHRONICLES_API UHintBaseWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	const FText& GetTitleText() const { return TitleText; }
	const FText& GetMainText() const { return MainText; }

	void SetTitleText(const FText& InText)
	{
		TitleText = InText;
		TitleTextBlock->SetText(TitleText);
	}
	void SetMainText(const FText& InText)
	{
		MainText = InText;
		MainTextBlock->SetText(MainText);
	}

	UTextBlock* GetTitleTextBlock() { return TitleTextBlock; }
	UTextBlock* GetMainTextBlock() {return MainTextBlock; }

protected:
	virtual void NativePreConstruct() override
	{
		Super::NativePreConstruct();

		TitleTextBlock->SetText(TitleText);
		MainTextBlock->SetText(MainText);
	}

private:
	UPROPERTY(EditDefaultsOnly)
	FText TitleText;

	UPROPERTY(EditDefaultsOnly)
	FText MainText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleTextBlock;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MainTextBlock;
};
