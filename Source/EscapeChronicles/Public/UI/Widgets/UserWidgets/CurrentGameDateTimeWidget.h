// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CurrentGameDateTimeWidget.generated.h"

class UCommonTextBlock;

struct FGameplayDateTime;

// A widget that displays the current day and time of the game
UCLASS()
class ESCAPECHRONICLES_API UCurrentGameDateTimeWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldGameDateTime,
		const FGameplayDateTime& NewGameDateTime);

private:
	// A widget that displays the current day of the game
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> DayTextWidget;

	// A widget that displays the current time of the game
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> TimeTextWidget;

	void SetCurrentGameDateTimeText(const FGameplayDateTime& CurrentGameDateTime) const;
};