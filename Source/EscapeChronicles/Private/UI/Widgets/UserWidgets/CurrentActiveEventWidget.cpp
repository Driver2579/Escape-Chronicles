// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/CurrentActiveEventWidget.h"

#include "CommonLazyImage.h"
#include "CommonTextBlock.h"
#include "EscapeChroniclesGameplayTags.h"
#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

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

	AEscapeChroniclesPlayerController* OwningPlayerController = GetOwningPlayer<AEscapeChroniclesPlayerController>();

	if (ensureAlways(IsValid(OwningPlayerController)))
	{
		OwningPlayerController->CallOrRegister_OnPlayerStateInitialized(
			FOnPlayerStateInitializedDelegate::FDelegate::CreateUObject(this,
				&ThisClass::OnOwningPlayerStateInitialized));
	}
}

void UCurrentActiveEventWidget::OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData)
{
	SetCurrentActiveEventText(NewEventData);

	// Hide the CheckedInStatusImageWidget if the current active event should hide it
	if (EventsHidingCheckedInStatusImageWidget.Contains(NewEventData.EventTag))
	{
		CheckedInStatusImageWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	// Otherwise, show it
	else
	{
		CheckedInStatusImageWidget->SetVisibility(ESlateVisibility::Visible);

		/**
		 * Update the checked-in status image based on the current checked-in status. This is needed because we have
		 * probably updated it while the widget was hidden. And we can't change the image while the widget is hidden.
		 */
		UpdateCheckedInStatusImage();
	}
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

void UCurrentActiveEventWidget::OnOwningPlayerStateInitialized(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

#if DO_CHECK
	check(IsValid(AbilitySystemComponent));
#endif

	// Listen for changes in the checked-in status of the player
	AbilitySystemComponent->RegisterAndCallGameplayTagEvent(EscapeChroniclesGameplayTags::Status_CheckedIn,
		FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(this, &ThisClass::OnCheckedInStatusChanged));
}

void UCurrentActiveEventWidget::OnCheckedInStatusChanged(FGameplayTag CheckedInTag, int32 TagCount)
{
	bCheckedIn = TagCount > 0;
	UpdateCheckedInStatusImage();
}

void UCurrentActiveEventWidget::UpdateCheckedInStatusImage() const
{
	const TSoftObjectPtr<UTexture2D>* CheckedInStatusImage = CheckedInStatusImages.Find(bCheckedIn);

	// Set the image for the current checked-in status if it exists
	if (ensureAlways(CheckedInStatusImage))
	{
#if DO_CHECK
		check(CheckedInStatusImageWidget);
#endif

#if DO_ENSURE
		ensureAlways(!CheckedInStatusImage->IsNull());
#endif

		CheckedInStatusImageWidget->SetBrushFromSoftTexture(*CheckedInStatusImage);
	}
	// Clear the image if no image was found for the current checked-in status
	else
	{
		CheckedInStatusImageWidget->SetBrushFromTexture(nullptr);
	}
}