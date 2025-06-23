// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "GameplayTagContainer.h"
#include "Common/Structs/ScheduleEventData.h"
#include "ScheduleEventsMusicManagerComponent.generated.h"

struct FStreamableHandle;

/**
 * A component that handles switching between different music tracks depending on the current ScheduleEvent. It also
 * contains an optional sound which, if specified, will be played before the music when the current active event is
 * switched, but only if it isn't the first time the music is requested to be played to avoid it being played at the
 * start of the game when the first event is active.
 */
UCLASS(ClassGroup=(Audio, Common), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UScheduleEventsMusicManagerComponent : public UAudioComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UScheduleEventsMusicManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
		const FScheduleEventData& NewEventData);

private:
	/**
	 * A map that associates each GameplayTag with a specific music track.
	 * @tparam KeyType The tag of the event.
	 * @tparam ValueType The sound that is played when the event with the specified tag is active.
	 */
	UPROPERTY(EditAnywhere, Category="Sound")
	TMap<FGameplayTag, TSoftObjectPtr<USoundBase>> EventsMusic;

	// An optional sound that is played when the current active event is switched
	UPROPERTY(EditAnywhere, Category="Sound")
	TSoftObjectPtr<USoundBase> SwitchEventSound;

	/**
	 * Sets the current music based on the active event and optionally plays the SwitchEventSound.
	 * @param CurrentEventTag The tag of the current event for which the music should be played.
	 * @param bPlaySwitchEventSound Whether to play the SwitchEventSound.
	 */
	void PlayMusicAndSoundForEvent(const FGameplayTag& CurrentEventTag, const bool bPlaySwitchEventSound = true);

	TSharedPtr<FStreamableHandle> LoadCurrentEventMusicHandle;
	TSharedPtr<FStreamableHandle> LoadSwitchEventSoundHandle;

	// Plays the current event music once if the SwitchEventSound isn't set of has been played already
	void OnCurrentEventMusicLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle, const bool bPlaySwitchEventSound);

	// Plays the SwitchEventSound and plays the current event music once the SwitchEventSound has finished playing
	void OnSwitchEventSoundLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle);

	TWeakObjectPtr<UAudioComponent> SwitchEventAudioComponent;
	FDelegateHandle OnSwitchEventSoundFinishedPlayingDelegateHandle;

	// Plays the current event music if it's loaded already
	void OnSwitchEventSoundFinishedPlaying(UAudioComponent* AudioComponent,
		TSharedPtr<FStreamableHandle> LoadSoundHandle);

	// Whether the SwitchEventSound has been played since the last event switch
	bool bSwitchEventSoundPlayed = false;

	// Whether the PlayMusicAndSoundForEvent function has been called at least once
	bool bPlayMusicAndSoundForEventCalledOnce = false;
};