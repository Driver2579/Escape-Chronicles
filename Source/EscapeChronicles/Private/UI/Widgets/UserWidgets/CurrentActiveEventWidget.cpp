// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/CurrentActiveEventWidget.h"

#include "CommonTextBlock.h"
#include "GameState/EscapeChroniclesGameState.h"

void UCurrentActiveEventWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	// Initialize the text for the current active event
	SetCurrentActiveEventText(GameState->GetCurrentActiveEventData());

	// Listen to current active event changes
	GameState->OnCurrentActiveEventChanged.AddUObject(this, &ThisClass::OnCurrentActiveEventChanged);
}

void UCurrentActiveEventWidget::OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData)
{
	SetCurrentActiveEventText(NewEventData);
}

void UCurrentActiveEventWidget::SetCurrentActiveEventText(const FScheduleEventData& CurrentActiveEventData) const
{
#if DO_CHECK
	check(EventNameTextWidget);
#endif

	const FText* EventName = EventsNames.Find(CurrentActiveEventData.EventTag);

	// Set the name of the current active event to the text widget if the event name is found
	if (EventName)
	{
		EventNameTextWidget->SetText(*EventName);
	}
	// Otherwise, set the text to an empty one
	else
	{
		EventNameTextWidget->SetText(FText::GetEmpty());
	}
}