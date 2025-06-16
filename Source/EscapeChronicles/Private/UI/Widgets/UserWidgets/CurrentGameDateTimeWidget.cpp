// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/CurrentGameDateTimeWidget.h"

#include "CommonTextBlock.h"
#include "GameState/EscapeChroniclesGameState.h"

void UCurrentGameDateTimeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	// Initialize the current GameDateTime
	SetCurrentGameDateTimeText(GameState->GetCurrentGameDateTime());

	// Listen to GameDateTime updates
	GameState->OnCurrentGameDateTimeUpdated.AddUObject(this, &ThisClass::OnCurrentGameDateTimeUpdated);
}

void UCurrentGameDateTimeWidget::OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldGameDateTime,
	const FGameplayDateTime& NewGameDateTime)
{
	SetCurrentGameDateTimeText(NewGameDateTime);
}

void UCurrentGameDateTimeWidget::SetCurrentGameDateTimeText(const FGameplayDateTime& CurrentGameDateTime) const
{
	// Set the text for the day
	DayTextWidget->SetText(FText::AsNumber(CurrentGameDateTime.Day));

	// Set the text for the time in 0:00 format
	TimeTextWidget->SetText(
		FText::FromString(
			FString::Printf(TEXT("%d:%02d"), CurrentGameDateTime.Time.Hour, CurrentGameDateTime.Time.Minute)));
}