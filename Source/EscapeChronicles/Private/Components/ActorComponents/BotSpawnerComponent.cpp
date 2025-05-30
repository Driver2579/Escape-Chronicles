// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/BotSpawnerComponent.h"

#include "AIController.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

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

	/**
	 * Usually, ReceiveControllerChangedDelegate (where this function is called from) is called before the pawn is set
	 * for the controller, even though it's already possessed at this stage. So we need to wait for the pawn to be set
	 * if it isn't already before restarting it.
	 */
	if (NewController->GetPawn() == Pawn)
	{
		OnControllerPossessedPawnChanged(nullptr, Pawn);
	}
	else
	{
		NewController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnControllerPossessedPawnChanged);
	}

	// Unsubscribe from the controller changes
	Pawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::OnCharacterControllerChanged);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UBotSpawnerComponent::OnControllerPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	// Don't do anything if this function was called for the pawn to be removed
	if (!IsValid(NewPawn))
	{
		return;
	}

#if DO_CHECK
	check(IsValid(NewPawn->GetController()));
#endif

#if DO_ENSURE
	ensureAlways(NewPawn->GetController()->IsA<AAIController>());
#endif

	AController* Controller = NewPawn->GetController();

	const UWorld* World = GetWorld();

	AEscapeChroniclesGameMode* GameMode = World->GetAuthGameMode<AEscapeChroniclesGameMode>();

	if (!ensureAlways(IsValid(GameMode)))
	{
		// Unsubscribe from the pawn changes if we ever subscribed to them
		Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::OnControllerPossessedPawnChanged);

		return;
	}

	/**
	 * Restart the AI controller. This will choose a PlayerStart for the character and move it there. If there was no
	 * character spawned, the default one will be spawned.
	 */
	if (ensureAlways(IsValid(GameMode)))
	{
		GameMode->RestartPlayer(Controller);
	}

	if (ensureAlways(Controller->PlayerState))
	{
#if DO_CHECK
		check(Controller->PlayerState->IsA<AEscapeChroniclesPlayerState>());
#endif

		AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(Controller->PlayerState);

		/**
		 * The bot is now spawned, restarted and possessed. Load and finish initializing it via the GameMode as
		 * required.
		 */
		GameMode->LoadAndInitBot(PlayerState);
	}

	// Unsubscribe from the pawn changes if we ever subscribed to them
	Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::OnControllerPossessedPawnChanged);
}