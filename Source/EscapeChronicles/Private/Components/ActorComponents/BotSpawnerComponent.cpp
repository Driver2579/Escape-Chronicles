// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/BotSpawnerComponent.h"

#include "AIController.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameModeBase.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "Subsystems/SaveGameSubsystem.h"

UBotSpawnerComponent::UBotSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBotSpawnerComponent::SpawnBots()
{
	// Get the StreamableManager once and use it for all the async loads
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

	// === Asynchronously load all classes and spawn them once they're loaded, but only if they have to be spawned ===

	if (PrisonerBotsCount > 0)
	{
#if DO_ENSURE
		ensureAlways(!PrisonerCharacterClass.IsNull());
#endif

		StreamableManager.RequestAsyncLoad(PrisonerCharacterClass.ToSoftObjectPath(),
			FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnPrisonerCharacterClassLoaded));
	}

	if (GuardBotsCount)
	{
#if DO_ENSURE
		ensureAlways(!GuardCharacterClass.IsNull());
#endif

		StreamableManager.RequestAsyncLoad(GuardCharacterClass.ToSoftObjectPath(),
			FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnGuardCharacterClassLoaded));
	}

	if (MedicBotsCount > 0)
	{
#if DO_ENSURE
		ensureAlways(!MedicCharacterClass.IsNull());
#endif

		StreamableManager.RequestAsyncLoad(MedicCharacterClass.ToSoftObjectPath(),
			FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnMedicCharacterClassLoaded));
	}
}

// ReSharper disable CppPassValueParameterByConstReference
void UBotSpawnerComponent::OnPrisonerCharacterClassLoaded(TSharedPtr<FStreamableHandle> LoadPrisonerClassesHandle) const
{
#if DO_CHECK
	check(LoadPrisonerClassesHandle.IsValid());
	check(PrisonerCharacterClass.IsValid());
#endif

	// Spawn the prisoner bots
	SpawnBots(PrisonerBotsCount, PrisonerCharacterClass.Get());

	// Release the handle to unload bots' classes when they're not used anymore
	LoadPrisonerClassesHandle->ReleaseHandle();
}

void UBotSpawnerComponent::OnGuardCharacterClassLoaded(TSharedPtr<FStreamableHandle> LoadGuardClassesHandle) const
{
#if DO_CHECK
	check(LoadGuardClassesHandle.IsValid());
	check(GuardCharacterClass.IsValid());
#endif

	// Spawn the guard bots
	SpawnBots(GuardBotsCount, GuardCharacterClass.Get());

	// Release the handle to unload bots' classes when they're not used anymore
	LoadGuardClassesHandle->ReleaseHandle();
}

void UBotSpawnerComponent::OnMedicCharacterClassLoaded(TSharedPtr<FStreamableHandle> LoadMedicClassesHandle) const
{
#if DO_CHECK
	check(LoadMedicClassesHandle.IsValid());
	check(MedicCharacterClass.IsValid());
#endif

	// Spawn the medic bots
	SpawnBots(MedicBotsCount, MedicCharacterClass.Get());

	// Release the handle to unload bots' classes when they're not used anymore
	LoadMedicClassesHandle->ReleaseHandle();
}
// ReSharper restore CppPassValueParameterByConstReference

void UBotSpawnerComponent::SpawnBots(const uint8 NumberOfBotsToSpawn,
	const TSubclassOf<AEscapeChroniclesCharacter>& CharacterClass) const
{
	for (uint8 i = 0; i < NumberOfBotsToSpawn; ++i)
	{
		SpawnOneBot(CharacterClass);
	}
}

void UBotSpawnerComponent::SpawnOneBot(const TSubclassOf<AEscapeChroniclesCharacter>& CharacterClass) const
{
#if DO_CHECK
	check(IsValid(CharacterClass));
#endif

	// We should always be able to spawn the character
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the character
	AEscapeChroniclesCharacter* Character = GetWorld()->SpawnActor<AEscapeChroniclesCharacter>(CharacterClass,
		SpawnParameters);

#if DO_CHECK
	check(IsValid(Character));
#endif

	// Wait for the AI controller to be initialized
	Character->ReceiveControllerChangedDelegate.AddDynamic(this, &ThisClass::OnCharacterControllerChanged);

	// Request the AI controller to spawn in case it isn't set to be spawned automatically
	Character->SpawnDefaultController();
}

void UBotSpawnerComponent::OnCharacterControllerChanged(APawn* Pawn, AController* OldController,
	AController* NewController)
{
	// Make sure the controller was initialized
	if (!IsValid(NewController))
	{
		return;
	}

#if DO_CHECK
	check(IsValid(Pawn));
#endif

#if DO_ENSURE
	// Make sure the controller is an AIController
	ensureAlways(NewController->IsA<AAIController>());
#endif

	const UWorld* World = GetWorld();

	AGameModeBase* GameMode = World->GetAuthGameMode();

	/**
	 * Restart the AI controller. This will choose a PlayerStart for the character and move it there. If there was no
	 * character spawned, the default one will be spawned.
	 */
	if (ensureAlways(IsValid(GameMode)))
	{
		GameMode->RestartPlayer(NewController);
	}

	/**
	 * Call the USaveGameSubsystem::LoadBotOrGenerateUniquePlayerID function for the bot because it's required to
	 * generate the UniquePlayerID for the bot and load the bot if it has any save data. But first we need to make sure
	 * the PlayerState is valid.
	 */
	if (ensureAlways(NewController->PlayerState))
	{
#if DO_CHECK
		check(NewController->PlayerState->IsA<AEscapeChroniclesPlayerState>());
#endif

		const USaveGameSubsystem* SaveGameSubsystem = World->GetSubsystem<USaveGameSubsystem>();

		if (ensureAlways(IsValid(SaveGameSubsystem)))
		{
			SaveGameSubsystem->LoadBotOrGenerateUniquePlayerID(
				CastChecked<AEscapeChroniclesPlayerState>(NewController->PlayerState));
		}
	}

	// Unsubscribe from the controller changes
	Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::OnCharacterControllerChanged);
}