// Fill out your copyright notice in the Description page of Project Settings.

#include "CheatManagers/EscapeChroniclesCheatManager.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/SharedRelationshipAttributeSet.h"
#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameInstances/EscapeChroniclesGameInstance.h"
#include "GameState/EscapeChroniclesGameState.h"

DEFINE_LOG_CATEGORY_STATIC(LogCheat, All, All);

void UEscapeChroniclesCheatManager::Cheat_SetGameDateTime(const uint64 Day, const uint8 Hour, const uint8 Minute) const
{
	const UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	AEscapeChroniclesGameState* GameState = World->GetGameState<AEscapeChroniclesGameState>();

	if (IsValid(GameState))
	{
		GameState->SetCurrentGameDateTime(FGameplayDateTime(Day, Hour, Minute));
	}
}

void UEscapeChroniclesCheatManager::Cheat_SetTimeDilation(const float TimeSpeed) const
{
	const UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	AWorldSettings* WorldSettings = World->GetWorldSettings();

	if (IsValid(WorldSettings))
	{
		WorldSettings->SetTimeDilation(TimeSpeed);
	}
}

void UEscapeChroniclesCheatManager::HostLevel(const FString& LevelPath) const
{
	const UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	UEscapeChroniclesGameInstance* GameInstance = World->GetGameInstance<UEscapeChroniclesGameInstance>();

	if (!IsValid(GameInstance))
	{
		return;
	}

	const APlayerController* OwningPlayerController = GetPlayerController();

	if (!IsValid(OwningPlayerController) || !OwningPlayerController->PlayerState)
	{
		return;
	}

	// Construct the LevelSoftObjectPtr from the LevelPath
	const FSoftObjectPath LevelSoftObjectPath(LevelPath);
	const TSoftObjectPtr<UWorld> LevelSoftObjectPtr(LevelSoftObjectPath);

	// Request to host a new session and send the LevelSoftObjectPtr to the callback
	GameInstance->StartHostSession(*OwningPlayerController->PlayerState->GetUniqueId().GetUniqueNetId(),
		FOnCreateSessionCompleteDelegate::CreateStatic(&ThisClass::OnCreateSessionComplete),
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete, LevelSoftObjectPtr));
}

void UEscapeChroniclesCheatManager::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogCheat, Warning,
			TEXT("UEscapeChroniclesCheatManager::OnCreateSessionComplete: Failed to create session!"));
	}
}

// ReSharper disable once CppPassValueParameterByConstReference
void UEscapeChroniclesCheatManager::OnStartSessionComplete(FName SessionName, bool bWasSuccessful,
	const TSoftObjectPtr<UWorld> LevelToServerTravel) const
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogCheat, Warning,
			TEXT("UEscapeChroniclesCheatManager::OnStartSessionComplete: Failed to start session!"));

		return;
	}

	UEscapeChroniclesGameInstance* GameInstance = GetWorld()->GetGameInstanceChecked<UEscapeChroniclesGameInstance>();

	// Open the level after the session was successfully created
	const bool bTravelResult = GameInstance->ServerTravelByLevelSoftObjectPtr(LevelToServerTravel);

	if (!bTravelResult)
	{
		UE_LOG(LogCheat, Warning,
			TEXT("UEscapeChroniclesCheatManager::OnStartSessionComplete: Failed to server travel to level "
				"%s!"),
			*LevelToServerTravel.GetLongPackageName());

		// Destroy the session if we failed
		GameInstance->DestroyHostSession();
	}
}

void UEscapeChroniclesCheatManager::EndHosting() const
{
	const UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return;
	}

	UEscapeChroniclesGameInstance* GameInstance = World->GetGameInstance<UEscapeChroniclesGameInstance>();

	if (!IsValid(GameInstance))
	{
		return;
	}

#if DO_ENSURE
	// Make sure we call this only on the server
	const APlayerController* OwningPlayerController = GetPlayerController();
	IsValid(OwningPlayerController) && ensureAlways(OwningPlayerController->HasAuthority());
#endif

	// Destroy the session and travel to the main menu
	GameInstance->DestroyHostSession(FOnDestroySessionCompleteDelegate(), true);
}

void UEscapeChroniclesCheatManager::Cheat_SetSuspicionBaseAttributeValue(const float NewBaseValue) const
{
	const AEscapeChroniclesPlayerController* PlayerController = Cast<AEscapeChroniclesPlayerController>(
		GetPlayerController());

	if (!IsValid(PlayerController))
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = PlayerController->GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	/**
	 * Try to get the USharedRelationshipAttributeSet from the AbilitySystemComponent to override its Suspicion base
	 * value.
	 */
	const USharedRelationshipAttributeSet* SharedRelationshipAttributeSet =
		CastChecked<USharedRelationshipAttributeSet>(
			AbilitySystemComponent->GetAttributeSet(USharedRelationshipAttributeSet::StaticClass()),
			ECastCheckedType::NullAllowed);

	// Override the base value of the attribute if the attribute set was found 
	if (ensureAlways(IsValid(SharedRelationshipAttributeSet)))
	{
		AbilitySystemComponent->SetNumericAttributeBase(SharedRelationshipAttributeSet->GetSuspicionAttribute(),
			NewBaseValue);
	}
}