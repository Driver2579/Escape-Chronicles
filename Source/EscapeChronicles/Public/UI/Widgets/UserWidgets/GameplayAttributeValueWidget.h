// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayAttributeValueWidget.generated.h"

class AEscapeChroniclesPlayerState;
class UCommonTextBlock;

// A widget that displays the current value of the specified gameplay attribute from owning player's ASC
UCLASS()
class ESCAPECHRONICLES_API UGameplayAttributeValueWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void OnOwningPlayerStateInitialized(AEscapeChroniclesPlayerState* PlayerState);

	virtual void OnAttributeValueChanged(const FOnAttributeChangeData& OnAttributeChangeData);

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> AttributeValueTextWidget;

	// A text that will be added at the end of the attribute value text
	UPROPERTY(EditAnywhere)
	FText AttributeValueTextPostfix;

	UPROPERTY(EditAnywhere)
	FGameplayAttribute AttributeToCapture;

	void SetAttributeValueText(const float NewValue) const;
};