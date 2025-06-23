// Fill out your copyright notice in the Description page of Project Settings.

#include "GameState/EscapeChroniclesGameState.h"

#include "Components/ActorComponents/ScheduleEventManagerComponent.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "HUDs/EscapeChroniclesHUD.h"
#include "Net/UnrealNetwork.h"

void AEscapeChroniclesGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentGameDateTime);

	DOREPLIFETIME(ThisClass, CurrentScheduledEventData);
	DOREPLIFETIME(ThisClass, CurrentActiveEventData);
}

void AEscapeChroniclesGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/**
	 * Initialize the CurrentGameTime in PostInitializeComponents to make sure we don't reinitialize it after the game
	 * is already loaded.
	 */
	CurrentGameDateTime = StartGameDateTime;
}

void AEscapeChroniclesGameState::BeginPlay()
{
	Super::BeginPlay();

	/**
	 * Start the timer to tick the game time for the first time both on server and clients to predict the time on
	 * clients.
	 */
	RestartTickGameTimeTimer();

	// Register the ScheduleEventManagerComponent data on the server
	RegisterScheduleEventManagerData();
}

void AEscapeChroniclesGameState::RegisterScheduleEventManagerData()
{
	if (!HasAuthority())
	{
		return;
	}

#if DO_CHECK
	check(AuthorityGameMode);
	check(AuthorityGameMode.IsA<AEscapeChroniclesGameMode>());
#endif

	const AEscapeChroniclesGameMode* EscapeChroniclesGameMode = CastChecked<AEscapeChroniclesGameMode>(
		AuthorityGameMode);

	UScheduleEventManagerComponent* ScheduleEventManagerComponent =
		EscapeChroniclesGameMode->GetScheduleEventManagerComponent();

#if DO_CHECK
	check(IsValid(ScheduleEventManagerComponent));
#endif

	// Synchronize current values with ScheduleEventManagerComponent
	CurrentScheduledEventData = ScheduleEventManagerComponent->GetCurrentScheduledEventData();
	CurrentActiveEventData = ScheduleEventManagerComponent->GetCurrentActiveEventData();

	// === Broadcast the delegates for the first time if the events are already set ===

	if (CurrentScheduledEventData.IsValid())
	{
		OnCurrentScheduledEventChanged.Broadcast(FScheduleEventData(), CurrentScheduledEventData);
	}

	if (CurrentActiveEventData.IsValid())
	{
		OnCurrentActiveEventChanged.Broadcast(FScheduleEventData(), CurrentActiveEventData);
	}

	// === Listen for events changes ===

	ScheduleEventManagerComponent->OnCurrentScheduledEventChanged.AddUObject(this,
		&ThisClass::NotifyCurrentScheduledEventChanged);

	ScheduleEventManagerComponent->OnCurrentActiveEventChanged.AddUObject(this,
		&ThisClass::NotifyCurrentActiveEventChanged);
}

void AEscapeChroniclesGameState::RestartTickGameTimeTimer()
{
	GetWorld()->GetTimerManager().SetTimer(TickGameTimeTimerHandle, this, &ThisClass::TickGameDateTime,
		GameTimeTickPeriod, true);
}

void AEscapeChroniclesGameState::SetCurrentGameDateTime(const FGameplayDateTime& NewGameDateTime)
{
	if (!HasAuthority())
	{
		return;
	}

	// Remember the old time and update the current time
	const FGameplayDateTime OldGameDateTime = CurrentGameDateTime;
	CurrentGameDateTime = NewGameDateTime;

	// Broadcast the changes
	OnCurrentGameDateTimeUpdated.Broadcast(OldGameDateTime, NewGameDateTime);
}

void AEscapeChroniclesGameState::TickGameDateTime()
{
	// Add one minute to the current time remembering the old one
	const FGameplayDateTime OldGameDateTime = CurrentGameDateTime++;

	// Broadcast the changes
	OnCurrentGameDateTimeUpdated.Broadcast(OldGameDateTime, CurrentGameDateTime);

	// TODO: Remove this once the UI is ready
	UE_LOG(LogTemp, Display, TEXT("Current game time: Day: %llu, Hour: %d, Minute: %d"),
		CurrentGameDateTime.Day, CurrentGameDateTime.Time.Hour, CurrentGameDateTime.Time.Minute);
}

void AEscapeChroniclesGameState::OnRep_CurrentDateTime(const FGameplayDateTime& OldValue)
{
	OnCurrentGameDateTimeUpdated.Broadcast(OldValue, CurrentGameDateTime);

	// Restart the tick GameTime timer to synchronize the update time with the server
	RestartTickGameTimeTimer();
}

void AEscapeChroniclesGameState::NotifyCurrentScheduledEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData)
{
#if DO_ENSURE
	ensureAlways(HasAuthority());
#endif

	// Update the CurrentScheduledEventData with a new value
	CurrentScheduledEventData = NewEventData;

	NetMulticast_BroadcastOnCurrentScheduledEventChangedDelegate(OldEventData, NewEventData);
}

void AEscapeChroniclesGameState::NotifyCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData)
{
#if DO_ENSURE
	ensureAlways(HasAuthority());
#endif

	// Update the CurrentActiveEventData with a new value
	CurrentActiveEventData = NewEventData;

	NetMulticast_BroadcastOnCurrentActiveEventChangedDelegate(OldEventData, NewEventData);
}

void AEscapeChroniclesGameState::NetMulticast_BroadcastOnCurrentScheduledEventChangedDelegate_Implementation(
	const FScheduleEventData& OldEventData, const FScheduleEventData& NewEventData)
{
	OnCurrentScheduledEventChanged.Broadcast(OldEventData, NewEventData);
}

void AEscapeChroniclesGameState::NetMulticast_BroadcastOnCurrentActiveEventChangedDelegate_Implementation(
	const FScheduleEventData& OldEventData, const FScheduleEventData& NewEventData)
{
	OnCurrentActiveEventChanged.Broadcast(OldEventData, NewEventData);
}

void AEscapeChroniclesGameState::OnPreLoadObject()
{
	// Remember the time before loading the game
	GameDateTimeBeforeLoading = MakeShared<FGameplayDateTime>(CurrentGameDateTime);
}

void AEscapeChroniclesGameState::OnPostLoadObject()
{
	// Restart the tick GameTime timer once we load the game
	RestartTickGameTimeTimer();

#if DO_CHECK
	check(GameDateTimeBeforeLoading.IsValid());
#endif

	// Broadcast the old and the current time to everyone after loading the game
	OnCurrentGameDateTimeUpdated.Broadcast(*GameDateTimeBeforeLoading, CurrentGameDateTime);

	// Clear the memory
	GameDateTimeBeforeLoading.Reset();
}

void AEscapeChroniclesGameState::NetMulticast_WinGame_Implementation()
{
	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	if (!ensureAlways(IsValid(Controller))) return;

	AEscapeChroniclesHUD* HUD = Controller->GetHUD<AEscapeChroniclesHUD>();
	if (ensureAlways(IsValid(HUD))) HUD->GoTo(GameWonHudRouteTag);

	Controller->SetPause(true);
}
