// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/SleepingManagementSubsystem.h"

#include "EngineUtils.h"
#include "Actors/ActivitySpot.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "GameFramework/GameModeBase.h"

bool USleepingManagementSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return this->GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer);
}

void USleepingManagementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	for (TActorIterator<AActor> It(&InWorld, BedClass.Get()); It; ++It)
	{
		AActivitySpot* Bed = Cast<AActivitySpot>(*It);

		if (!IsValid(Bed))
		{
			continue;
		}
		
		Bed->OnOccupyingStateChanged.AddUObject(this, &ThisClass::OnBedOccupyingCharacterChanged);

		Beds.Add(Bed);
	}
	
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnGameModePostLogin);
	FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &ThisClass::OnGameModeLogout);

	UpdateTimeSpeed();
}

void USleepingManagementSubsystem::OnBedOccupyingCharacterChanged(AEscapeChroniclesCharacter* Character) const
{
	// Updating as the total number of players has changed
	UpdateTimeSpeed();
}

void USleepingManagementSubsystem::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer) const
{
	// Updating as the total number of players has changed
	UpdateTimeSpeed();
}

void USleepingManagementSubsystem::OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting) const
{
	// Updating as the total number of players has changed
	UpdateTimeSpeed();
}

int32 USleepingManagementSubsystem::GetSleepingPlayersNumber() const
{
	int32 Result = 0;

	for (const AActivitySpot* Bed : Beds)
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
	if (GetWorld()->GetNumPlayerControllers() == GetSleepingPlayersNumber())
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(SleepTimeDilation);
	}
	else
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(1);
	}
}
