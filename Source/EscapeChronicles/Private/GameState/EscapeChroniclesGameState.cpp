// Fill out your copyright notice in the Description page of Project Settings.

#include "GameState/EscapeChroniclesGameState.h"

#include "Net/UnrealNetwork.h"

void AEscapeChroniclesGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentGameDateTime);
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
}

void AEscapeChroniclesGameState::RestartTickGameTimeTimer()
{
	GetWorld()->GetTimerManager().SetTimer(TickGameTimeTimerHandle, this, &ThisClass::TickGameDateTime,
		GameTimeTickPeriod, true);
}

void AEscapeChroniclesGameState::SetCurrentGameDateTime(const FGameplayDateTime& NewGameTime)
{
	if (HasAuthority())
	{
		CurrentGameDateTime = NewGameTime;
		OnCurrentDateTimeUpdated.Broadcast(NewGameTime);
	}
}

void AEscapeChroniclesGameState::TickGameDateTime()
{
	++CurrentGameDateTime;
	OnCurrentDateTimeUpdated.Broadcast(CurrentGameDateTime);

	// TODO: Remove this once the UI is ready
	UE_LOG(LogTemp, Display, TEXT("Current game time: Day: %d, Hour: %d, Minute: %d"),
		CurrentGameDateTime.Day, CurrentGameDateTime.Time.Hour, CurrentGameDateTime.Time.Minute);
}

void AEscapeChroniclesGameState::OnPreLoadObject()
{
	// Restart tht tick GameTime timer once we load the game
	RestartTickGameTimeTimer();

	// Broadcast the current time to everyone after loading the game
	OnCurrentDateTimeUpdated.Broadcast(CurrentGameDateTime);
}

void AEscapeChroniclesGameState::OnRep_CurrentDateTime()
{
	// Restart the tick GameTime timer to synchronize the update time with the server
	RestartTickGameTimeTimer();

	OnCurrentDateTimeUpdated.Broadcast(CurrentGameDateTime);
}