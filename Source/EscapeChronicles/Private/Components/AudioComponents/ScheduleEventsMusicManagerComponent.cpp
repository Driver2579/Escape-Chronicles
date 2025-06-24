// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AudioComponents/ScheduleEventsMusicManagerComponent.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "Kismet/GameplayStatics.h"

UScheduleEventsMusicManagerComponent::UScheduleEventsMusicManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Play the music in 2D
	bAllowSpatialization = false;

	// We want to play the music when the game is paused
	bIsUISound = true;

	// Support music virtualization to make sure it isn't destroyed when the game is paused or the music volume is 0
	bIsVirtualized = true;
}

void UScheduleEventsMusicManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	const UWorld* World = GetWorld();

	// Don't run this component on dedicated servers because sounds won't work there anyway
	if (World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AEscapeChroniclesGameState* GameState = World->GetGameState<AEscapeChroniclesGameState>();

	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	/**
	 * If the current active event is valid already, then play the music for it but don't play the SwitchEventSound
	 * because it's the first time the music is played.
	 */
	PlayMusicAndSoundForEvent(GameState->GetCurrentActiveEventData().EventTag, false);

	GameState->OnCurrentActiveEventChanged.AddUObject(this, &ThisClass::OnCurrentActiveEventChanged);
}

void UScheduleEventsMusicManagerComponent::OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData)
{
	/**
	 * Play the music for the new event but play the SwitchEventSound only if we already called this function at least
	 * once.
	 */
	PlayMusicAndSoundForEvent(NewEventData.EventTag, bPlayMusicAndSoundForEventCalledOnce);
}

void UScheduleEventsMusicManagerComponent::PlayMusicAndSoundForEvent(const FGameplayTag& CurrentEventTag,
	const bool bPlaySwitchEventSound)
{
	if (!CurrentEventTag.IsValid())
	{
		return;
	}

	// Reset the flag because the new event has started
	bSwitchEventSoundPlayed = false;

	// Stop the SwitchEventSound if it's currently playing and release the pointer to its component
	if (SwitchEventAudioComponent.IsValid())
	{
		/**
		 * First, we want to unsubscribe from the event called when the sound finishes playing to make sure it isn't
		 * called when the sound is stopped. Then we stop the sound.
		 */
		SwitchEventAudioComponent->OnAudioFinishedNative.Remove(OnSwitchEventSoundFinishedPlayingDelegateHandle);
		SwitchEventAudioComponent->Stop();

		SwitchEventAudioComponent.Reset();
	}

	// Unload or cancel loading the previous music if any
	if (LoadCurrentEventMusicHandle.IsValid())
	{
		LoadCurrentEventMusicHandle->CancelHandle();
		LoadCurrentEventMusicHandle.Reset();
	}

	// Load the sound that should be played when the event is switched but only if it is set and if it was requested
	if (bPlaySwitchEventSound && !SwitchEventSound.IsNull())
	{
		// Load the sound only if its loading wasn't already requested
		if (!LoadSwitchEventSoundHandle.IsValid())
		{
			LoadSwitchEventSoundHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				SwitchEventSound.ToSoftObjectPath(),
				FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnSwitchEventSoundLoaded));
		}
		// If the sound is already loaded, we can just play it immediately
		else if (SwitchEventSound.IsValid())
		{
			OnSwitchEventSoundLoaded(LoadSwitchEventSoundHandle);
		}
	}

	// Stop the music that is currently playing if any and remove it from this component
	Stop();
	SetSound(nullptr);

	// Try to find the music for the new event
	const TSoftObjectPtr<USoundBase>* CurrentEventMusic = EventsMusic.Find(CurrentEventTag);

	// Load the music for the current event if it's set
	if (CurrentEventMusic && ensureAlways(!CurrentEventMusic->IsNull()))
	{
		LoadCurrentEventMusicHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			CurrentEventMusic->ToSoftObjectPath(),
			FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnCurrentEventMusicLoaded,
				bPlaySwitchEventSound));
	}

	// Remember that this function has been called at least once
	bPlayMusicAndSoundForEventCalledOnce = true;
}

// ReSharper disable once CppPassValueParameterByConstReference
void UScheduleEventsMusicManagerComponent::OnCurrentEventMusicLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
	const bool bPlaySwitchEventSound)
{
#if DO_CHECK
	check(LoadObjectHandle.IsValid());

	check(IsValid(LoadObjectHandle->GetLoadedAsset()));
	check(LoadObjectHandle->GetLoadedAsset()->IsA<USoundBase>());
#endif

	USoundBase* LoadedMusic = CastChecked<USoundBase>(LoadObjectHandle->GetLoadedAsset());

	/**
	 * If we either don't want to wait for the SwitchEventSound to be played or shouldn't play it or it isn't set, then
	 * play the loaded music immediately. Otherwise, check if the SwitchEventSound has finished playing and don't play
	 * the music yet if it hasn't. We will play the music once this sound has finished playing in that case.
	 */
	const bool bPlayMusicImmediately = !bWaitSwitchEventSoundEndBeforePlayingMusic || !bPlaySwitchEventSound ||
		SwitchEventSound.IsNull() || bSwitchEventSoundPlayed;

	if (bPlayMusicImmediately)
	{
		SetSound(LoadedMusic);
		Play();
	}
}

void UScheduleEventsMusicManagerComponent::OnSwitchEventSoundLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle)
{
#if DO_CHECK
	check(LoadObjectHandle.IsValid());
	check(SwitchEventSound.IsValid());
#endif

	SwitchEventAudioComponent = UGameplayStatics::CreateSound2D(this, SwitchEventSound.Get());

#if DO_CHECK
	check(SwitchEventAudioComponent.IsValid());
#endif

	// Support sound virtualization to make sure it isn't destroyed when the game is paused or the sound volume is 0
	SwitchEventAudioComponent->bIsVirtualized = true;

	/**
	 * Listen for the sound to finish playing if we need to wait for it to finish before playing the music. This will
	 * also release the handle once the sound has finished playing.
	 */
	if (bWaitSwitchEventSoundEndBeforePlayingMusic)
	{
		OnSwitchEventSoundFinishedPlayingDelegateHandle = SwitchEventAudioComponent->OnAudioFinishedNative.AddUObject(
			this, &ThisClass::OnSwitchEventSoundFinishedPlaying, LoadObjectHandle);
	}
	// Otherwise, release the handle immediately because we don't need it anymore
	else
	{
		LoadObjectHandle->ReleaseHandle();
	}

	SwitchEventAudioComponent->Play();
}

void UScheduleEventsMusicManagerComponent::OnSwitchEventSoundFinishedPlaying(UAudioComponent* AudioComponent,
	TSharedPtr<FStreamableHandle> LoadSoundHandle)
{
	/**
	 * Remember that the SwitchEventSound has been played so we can play the music immediately once it's loaded if it
	 * isn't already.
	 */
	bSwitchEventSoundPlayed = true;

#if DO_CHECK
	check(LoadSoundHandle.IsValid());
#endif

	// Unload the sound because it has been played already
	LoadSoundHandle->ReleaseHandle();
	LoadSoundHandle.Reset();

	// Don't play the music if it isn't loaded yet (or if it isn't loading at all)
	if (!LoadCurrentEventMusicHandle.IsValid() || LoadCurrentEventMusicHandle->IsLoadingInProgress())
	{
		return;
	}

#if DO_CHECK
	check(IsValid(LoadCurrentEventMusicHandle->GetLoadedAsset()));
	check(LoadCurrentEventMusicHandle->GetLoadedAsset()->IsA<USoundBase>());
#endif

	USoundBase* CurrentEventMusic = CastChecked<USoundBase>(LoadCurrentEventMusicHandle->GetLoadedAsset());

	// Play the current event music if it's loaded already
	SetSound(CurrentEventMusic);
	Play();
}