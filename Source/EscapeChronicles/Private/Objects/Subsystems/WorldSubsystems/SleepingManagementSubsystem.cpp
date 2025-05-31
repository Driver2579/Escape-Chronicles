// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/Subsystems/WorldSubsystems/SleepingManagementSubsystem.h"

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
	
	//AActivitySpot BedSpotClass
	
	for (TActorIterator<AActor> It(&InWorld, BedSpotClass.Get()); It; ++It)
	{
		AActivitySpot* Bed = Cast<AActivitySpot>(*It);

		if (!IsValid(Bed))
		{
			continue;
		}

		Beds.Add(Bed);
		
		Bed->AddOccupyingCharacterChangedHandler(
			AActivitySpot::FOnOccupyingCharacterChanged::FDelegate::CreateUObject(this,
				&ThisClass::OnBedOccupyingCharacterChanged));
	}

	DefaultTimeDilation = InWorld.GetWorldSettings()->TimeDilation;
	
	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnGameModePostLogin);
	FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &ThisClass::OnGameModeLogout);

	NetMulticast_UpdateTimeSpeed_Implementation();
}

void USleepingManagementSubsystem::OnBedOccupyingCharacterChanged(AEscapeChroniclesCharacter* Character)
{
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		// Updating as the total number of players has changed
		NetMulticast_UpdateTimeSpeed_Implementation();
	}
}

void USleepingManagementSubsystem::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		// Updating as the total number of players has changed
		NetMulticast_UpdateTimeSpeed_Implementation();
	}
}

void USleepingManagementSubsystem::OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting)
{
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		// Updating as the total number of players has changed
		NetMulticast_UpdateTimeSpeed_Implementation();
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

void USleepingManagementSubsystem::NetMulticast_UpdateTimeSpeed_Implementation() const
{
	if (GetWorld()->GetNumPlayerControllers() == GetSleepingPlayersNumber())
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(SleepTimeDilation);
	}
	else
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(DefaultTimeDilation);
	}
}
