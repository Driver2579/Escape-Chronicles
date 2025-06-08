// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/Subsystems/WorldSubsystems/SleepingManagementSubsystem.h"

#include "EngineUtils.h"
#include "Actors/ActivitySpot.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "GameState/EscapeChroniclesGameState.h"

bool USleepingManagementSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return this->GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer);
}

void USleepingManagementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	
	for (TActorIterator<AActor> It(&InWorld, BedClass.Get()); It; ++It)
	{
		AActivitySpot* Bed = Cast<AActivitySpot>(*It);

		if (!IsValid(Bed))
		{
			continue;
		}
		
		Bed->AddOccupyingCharacterChangedHandler(
			AActivitySpot::FOnOccupyingCharacterChanged::FDelegate::CreateUObject(this,
				&ThisClass::OnBedOccupyingCharacterChanged));

		Beds.Add(Bed);
	}

	AEscapeChroniclesGameState* GameState = InWorld.GetGameState<AEscapeChroniclesGameState>();

	if (IsValid(GameState))
	{
		// Initialize the CurrentActiveEventTag with the tag from... the current active event 🤩
		CurrentActiveEventTag = GameState->GetCurrentActiveEventData().EventTag;

		// Listen for active event changes
		GameState->OnCurrentActiveEventChanged.AddUObject(this, &ThisClass::OnCurrentActiveEventChanged);
	}

	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnGameModePostLogin);
	FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &ThisClass::OnGameModeLogout);

	UpdateTimeSpeed();
}

void USleepingManagementSubsystem::OnBedOccupyingCharacterChanged(AEscapeChroniclesCharacter* Character)
{
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		// Updating as the total number of players has changed
		UpdateTimeSpeed();
	}
}

void USleepingManagementSubsystem::OnCurrentActiveEventChanged(const FScheduleEventData& OldEventData,
	const FScheduleEventData& NewEventData)
{
	// Remember the current event
	CurrentActiveEventTag = NewEventData.EventTag;

	// Try to update the time dilation since the criteria for it have changed
	UpdateTimeSpeed();
}

void USleepingManagementSubsystem::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		// Updating as the total number of players has changed
		UpdateTimeSpeed();
	}
}

void USleepingManagementSubsystem::OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting)
{
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		// Updating as the total number of players has changed
		UpdateTimeSpeed();
	}
}

int32 USleepingManagementSubsystem::GetSleepingPlayersNumber() const
{
	int32 Result = 0;

	for (AActivitySpot* Bed : Beds)
	{
		const AEscapeChroniclesCharacter* Character = Bed->GetOccupyingCharacter();

		if (IsValid(Character) && IsValid(Character->GetController<APlayerController>()))
		{
			Result += 1;
		}
	}

	return Result;
}

void USleepingManagementSubsystem::UpdateTimeSpeed() const
{
	// Check if all the players are in their beds and if the current active event allows changing the time dilation
	const bool bCanChangeTimeDilation = GetWorld()->GetNumPlayerControllers() == GetSleepingPlayersNumber() &&
		RequiredEventsToChangeTimeDilation.HasTagExact(CurrentActiveEventTag);

	if (bCanChangeTimeDilation)
	{
		NetMulticast_SetTimeDilation(SleepTimeDilation);
	}
	else
	{
		NetMulticast_SetTimeDilation(1);
	}
}

void USleepingManagementSubsystem::NetMulticast_SetTimeDilation_Implementation(const float InTimeDilation) const
{
	GetWorld()->GetWorldSettings()->SetTimeDilation(InTimeDilation);
}
