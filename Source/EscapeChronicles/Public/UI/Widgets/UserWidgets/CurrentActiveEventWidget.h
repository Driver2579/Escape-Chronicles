// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Common/Structs/ScheduleEventData.h"
#include "CurrentActiveEventWidget.generated.h"

class AEscapeChroniclesPlayerState;
class UCommonLazyImage;
class UCommonTextBlock;

// A widget that displays the name of the current active event in the game
UCLASS()
class ESCAPECHRONICLES_API UCurrentActiveEventWidget : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData);

	virtual void OnOwningPlayerStateInitialized(AEscapeChroniclesPlayerState* PlayerState);

	virtual void OnCheckedInStatusChanged(FGameplayTag CheckedInTag, int32 TagCount);

private:
	// A text that displays the current active event name
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonTextBlock> EventNameTextWidget;

	/**
	 * The names that will be displayed for each event. All events should have a name in this map.
	 * @tparam KeyType Tag of the event.
	 * @tparam ValueType Text that will be displayed for the event.
	 */
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, FText> EventsNames;

	void SetCurrentActiveEventText(const FScheduleEventData& CurrentActiveEventData) const;

	// An image that displays whether the player has checked in during the current active event
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UCommonLazyImage> CheckedInStatusImageWidget;

	// Images that will be displayed for the CheckedInStatusImageWidget depending on whether the player has checked in
	UPROPERTY(EditAnywhere)
	TMap<bool, TSoftObjectPtr<UTexture2D>> CheckedInStatusImages;

	// Tags of events that should hide the checked-in status image widget when they are active
	UPROPERTY(EditAnywhere)
	TSet<FGameplayTag> EventsHidingCheckedInStatusImageWidget;

	bool bCheckedIn = false;

	// Updates the CheckedInStatusImages depending on whether the player has checked in
	void UpdateCheckedInStatusImage() const;
};