// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/EscapeChroniclesPlayerState.h"

#include "Common/DataAssets/AbilitySystemSet.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "Net/UnrealNetwork.h"

AEscapeChroniclesPlayerState::AEscapeChroniclesPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UEscapeChroniclesAbilitySystemComponent>(
		TEXT("AbilitySystemComponent"));

	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AEscapeChroniclesPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, UniquePlayerID);
}

void AEscapeChroniclesPlayerState::InitPlayerStateForController(AController* OwnerController,
	const TSubclassOf<AEscapeChroniclesPlayerState>& PlayerStateClass)
{
	// The next code was mostly copied from AController::InitPlayerState() but with some modifications

#if DO_CHECK
	check(IsValid(OwnerController));
	check(IsValid(PlayerStateClass));
#endif

	if (OwnerController->GetNetMode() == NM_Client || !ensureAlways(IsValid(GEngine)))
	{
		return;
	}

	UWorld* World = OwnerController->GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return;
	}

	const AGameModeBase* GameMode = World->GetAuthGameMode();

	/**
	 * If the GameMode is null, this might be a network client that's trying to record a replay. Try to use the default
	 * game mode in this case so that we can still spawn a PlayerState.
	 */
	if (!IsValid(GameMode))
	{
		const AGameStateBase* GameState = World->GetGameState();

		if (IsValid(GameState))
		{
			GameMode = GameState->GetDefaultGameMode();

			if (!IsValid(GameMode))
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = OwnerController;
	SpawnInfo.Instigator = OwnerController->GetInstigator();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient; // We never want player states to save into a map

	OwnerController->SetPlayerState(World->SpawnActor<AEscapeChroniclesPlayerState>(PlayerStateClass, SpawnInfo));

	// Force a default player name if necessary
	if (OwnerController->PlayerState && OwnerController->PlayerState->GetPlayerName().IsEmpty())
	{
		/**
		 * Don't call SetPlayerName() as that will broadcast entry messages but the GameMode hasn't had a chance to
		 * potentially apply a player/bot name yet.
		 */
		OwnerController->PlayerState->SetPlayerNameInternal(GameMode->DefaultPlayerName.ToString());
	}
}

void AEscapeChroniclesPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/**
	 * Give AbilitySystemSets to the AbilitySystemComponent. We do this here because SetupInputComponent is called
	 * before BeginPlay. Except we don't want to apply gameplay effects before the pawn is set to be able to initialize
	 * attributes with data from the pawn.
	 */
	for (UAbilitySystemSet* AbilitySystemSet : AbilitySystemSets)
	{
		if (AbilitySystemSet)
		{
			AbilitySystemSet->GiveAttributesToAbilitySystem(AbilitySystemComponent);
			AbilitySystemSet->GiveAbilitiesToAbilitySystem(AbilitySystemComponent);
		}
	}

	OnPawnSet.AddDynamic(this, &ThisClass::OnPawnChanged);
}

void AEscapeChroniclesPlayerState::OnPawnChanged(APlayerState* ThisPlayerState, APawn* NewPawn, APawn* OldPawn)
{
	// Don't do anything if the new pawn is invalid (for example, if it was removed instead of being set)
	if (!IsValid(NewPawn))
	{
		return;
	}

	// Don't reapply effects if the new pawn is a spectator
	if (NewPawn->IsA<ASpectatorPawn>())
	{
		// Remember the last pawn that wasn't a spectator
		LastNotSpectatorPawn = OldPawn;

		return;
	}

	/**
	 * Don't reapply effects if the new pawn is the same as the last pawn that wasn't a spectator (for example, if we
	 * had the pawn, then switched to a spectator, then switched back to the pawn).
	 */
	if (LastNotSpectatorPawn.IsValid() && NewPawn == LastNotSpectatorPawn)
	{
		// Forget about the last pawn that wasn't a spectator
		LastNotSpectatorPawn.Reset();

		return;
	}

	// Initialize attributes BEFORE the gameplay effects are applied. They may want to use attributes.
	InitializeAttributes();

	/**
	 * Reapply (or apply for the first time) effects from AbilitySystemSets to the AbilitySystemComponent when the new
	 * pawn is set.
	 */
	for (UAbilitySystemSet* AbilitySystemSet : AbilitySystemSets)
	{
		if (AbilitySystemSet)
		{
			AbilitySystemSet->TakeEffectsFromAbilitySystem(AbilitySystemComponent);
			AbilitySystemSet->GiveEffectsToAbilitySystem(AbilitySystemComponent);
		}

		// We also want to apply blocking attributes by tags only after the gameplay effects are applied
		AbilitySystemSet->ApplyBlockingAttributesByTags(AbilitySystemComponent);
	}
}

bool AEscapeChroniclesPlayerState::IsOnlinePlayer() const
{
	// If it's a bot, then it's for sure not a player
	if (IsABot())
	{
		return false;
	}

	const FUniqueNetIdRepl& UniqueNetId = GetUniqueId();

#if DO_ENSURE
	ensureAlways(UniqueNetId.IsValid());
#endif

	return (UniqueNetId.IsV1() && !UniqueNetId.GetV1Unsafe()->GetType().IsEqual(TEXT("NULL"))) ||
		(UniqueNetId.IsV2() && UniqueNetId.GetV2Unsafe().GetOnlineServicesType() != UE::Online::EOnlineServices::None);
}

void AEscapeChroniclesPlayerState::GenerateUniquePlayerIdIfInvalid()
{
	if (UniquePlayerID.IsValid())
	{
		return;
	}

	const UWorld* World = GetWorld();

	AEscapeChroniclesGameMode* GameMode = World->GetAuthGameMode<AEscapeChroniclesGameMode>();

	if (!ensureAlways(GameMode))
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	UniquePlayerID = GameMode->GetUniquePlayerIdManager().GenerateUniquePlayerIdForPIE();
#else
	// We don't currently support split-screen, so always use 0 as the LocalPlayerID in the build
	UniquePlayerID = GameMode->GetUniquePlayerIdManager().GenerateUniquePlayerID(0);
#endif

	// Generate the NetID if it's an online player
	if (IsOnlinePlayer())
	{
		UniquePlayerID.NetID = GetUniqueId()->ToString();
	}
}