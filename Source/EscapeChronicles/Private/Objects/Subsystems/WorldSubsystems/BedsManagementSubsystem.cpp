// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/Subsystems/WorldSubsystems/BedsManagementSubsystem.h"

#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"

void UBedsManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DefaultTimeDilation = GetWorld()->GetWorldSettings()->TimeDilation;

	FGameModeEvents::GameModePostLoginEvent.AddUObject(this, &ThisClass::OnGameModePostLogin);
	FGameModeEvents::GameModeLogoutEvent.AddUObject(this, &ThisClass::OnGameModeLogout);

	NetMulticast_UpdateTimeSpeed_Implementation();
}

void UBedsManagementSubsystem::IncreaseSleepingPlayersNumber()
{
	// Change the value on the server, then it is replicated
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		++SleepingPlayersNumber;
		NetMulticast_UpdateTimeSpeed_Implementation();
	}
}

void UBedsManagementSubsystem::DecreaseSleepingPlayersNumber()
{
	// Change the value on the server, then it is replicated
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		--SleepingPlayersNumber;
		NetMulticast_UpdateTimeSpeed_Implementation();
	}
}

void UBedsManagementSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SleepingPlayersNumber);
}

bool UBedsManagementSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return this->GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer);
}

void UBedsManagementSubsystem::OnGameModePostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	// Updating as the total number of players has changed
	NetMulticast_UpdateTimeSpeed_Implementation();
}

void UBedsManagementSubsystem::OnGameModeLogout(AGameModeBase* GameMode, AController* Exiting)
{
	// Updating as the total number of players has changed
	NetMulticast_UpdateTimeSpeed_Implementation();
}

void UBedsManagementSubsystem::NetMulticast_UpdateTimeSpeed_Implementation() const
{
	if (GetWorld()->GetNumPlayerControllers() == SleepingPlayersNumber)
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(SleepTimeDilation);
	}
	else
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(DefaultTimeDilation);
	}
}
