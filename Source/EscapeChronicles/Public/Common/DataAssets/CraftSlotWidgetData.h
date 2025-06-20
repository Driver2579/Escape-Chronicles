// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/Widgets/UserWidgets/HintBaseWidget.h"
#include "CraftSlotWidgetData.generated.h"

class UInformPopup;
class UConfirmationPopup;

UCLASS()
class ESCAPECHRONICLES_API UCraftSlotWidgetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush InvalidItemInstanceBrush;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UConfirmationPopup> ConfirmationPopupClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInformPopup> InformPopupClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHintBaseWidget> ToolTipWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	FText CraftIsNotPossibleInformText;

	UPROPERTY(EditDefaultsOnly)
	FText CraftConfirmationText;

	UPROPERTY(Transient)
	TObjectPtr<UHintBaseWidget> ToolTipWidget;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		if (ensureAlways(IsValid(ToolTipWidgetClass)))
		{
			ToolTipWidget = NewObject<UHintBaseWidget>(this, ToolTipWidgetClass.Get());
		}
	}
#endif

	virtual void PostLoad() override
	{
		Super::PostLoad();

		if (IsValid(ToolTipWidgetClass))
		{
			ToolTipWidget = NewObject<UHintBaseWidget>(this, ToolTipWidgetClass.Get());
		}
	}
};