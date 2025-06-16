// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/BotSpawnerComponent.h"

#include "AIController.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/DataAssets/BotNames.h"
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

#if DO_CHECK
	check(NewPawn->IsA<AEscapeChroniclesCharacter>());
#endif

	const AEscapeChroniclesCharacter* Character = CastChecked<AEscapeChroniclesCharacter>(NewPawn);

	/**
	 * TODO:
	 * There is a bug in the engine that makes the UWorld::EncroachingBlockingGeometry function (or at least I think
	 * it's because of it) not work correctly if the actor has any components except of the RootComponent that have
	 * collisions enabled. The issue is that it completely ignores collision responses on such components, so even
	 * though the mesh component ignores the capsule, the engine still thinks that the mesh is colliding with capsule
	 * which results that the engine thinks that the character is colliding with something even if it doesn't. So the
	 * solution would be to disable the collision for the mesh component before restarting the character and then enable
	 * it back after the character is restarted. Otherwise, the PlayerStart selection won't work correctly because the
	 * engine would always think that the character would collide with something when choosing the PlayerStart, and the
	 * character will be spawned at ZeroLocation instead.
	 */
	const FName DefaultMeshCollisionProfileName = Character->GetMesh()->GetCollisionProfileName();
	Character->GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	/**
	 * Restart the AI controller. This will choose a PlayerStart for the character and move it there. If there was no
	 * character spawned, the default one will be spawned.
	 */
	if (ensureAlways(IsValid(GameMode)))
	{
		GameMode->RestartPlayer(Controller);
	}

	Character->GetMesh()->SetCollisionProfileName(DefaultMeshCollisionProfileName);

	if (ensureAlways(Controller->PlayerState))
	{
#if DO_CHECK
		check(Controller->PlayerState->IsA<AEscapeChroniclesPlayerState>());
#endif

		AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(Controller->PlayerState);

		// Select the name for this bot
		FString BotName;
		SelectBotName(BotName);

		// Set the name of the bot to the PlayerState. It will be overriden by the save if it was saved
		PlayerState->SetPlayerName(BotName);

		/**
		 * The bot is now spawned, restarted, possessed and has a name. Load and finish initializing it via the GameMode
		 * as required.
		 */
		GameMode->LoadAndInitBot(PlayerState);
	}

	// Unsubscribe from the pawn changes if we ever subscribed to them
	Controller->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::OnControllerPossessedPawnChanged);
}

void UBotSpawnerComponent::SelectBotName(FString& OutName)
{
	if (!ensureAlways(BotNamesDataAsset))
	{
		return;
	}

	// Get all unclaimed names
	TSet<FString> SelectableBotNames = BotNamesDataAsset->GetBotNames().Difference(ClaimedBotNames);

	// Use any bot name if there are no unclaimed names left
	if (SelectableBotNames.IsEmpty())
	{
		SelectableBotNames = BotNamesDataAsset->GetBotNames();
	}

	// Select a random index for the name
	int32 RandomIndex = FMath::RandRange(0, SelectableBotNames.Num() - 1);

	// Iterate through the names and select the one at the random index
	for (const FString& Name : SelectableBotNames)
	{
		/**
		 * Decrement the index until it reaches zero. This will make sure we iterate to the needed index (because there
		 * are no indexes in TSet).
		 */
		if (RandomIndex-- > 0)
		{
			continue;
		}

		// Return the name and remember that we claimed it
		OutName = Name;
		ClaimedBotNames.Add(Name);

		break;
	}
}