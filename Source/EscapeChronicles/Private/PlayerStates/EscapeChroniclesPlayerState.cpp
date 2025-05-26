// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerStates/EscapeChroniclesPlayerState.h"

#include "EscapeChroniclesGameplayTags.h"
#include "Common/DataAssets/AbilitySystemSet.h"
#include "Common/Enums/CharacterRole.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
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
	 * before BeginPlay.
	 */
	for (UAbilitySystemSet* AbilitySystemSet : AbilitySystemSets)
	{
		if (AbilitySystemSet)
		{
			AbilitySystemSet->GiveToAbilitySystem(AbilitySystemComponent);
		}
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

	// Players and bots have different LocalPlayerIDs
	if (!IsABot())
	{
#if WITH_EDITOR
		UniquePlayerID = GameMode->GetUniquePlayerIdManager().GenerateUniquePlayerIdForPIE();
#else
		// We don't currently support split-screen, so always use 0 as the LocalPlayerID in the build
		UniquePlayerID = GameMode->GetUniquePlayerIdManager().GenerateUniquePlayerID(0);
#endif
	}
	else
	{
		// Always use 0 for LocalPlayerIDs of the bots
		UniquePlayerID = GameMode->GetUniquePlayerIdManager().GenerateUniquePlayerID(0);
	}

	// Generate the NetID if it's an online player
	if (IsOnlinePlayer())
	{
		UniquePlayerID.NetID = GetUniqueId()->ToString();
	}

	OnUniquePlayerIdInitializedOrChanged();
}

void AEscapeChroniclesPlayerState::SetUniquePlayerID(const FUniquePlayerID& NewUniquePlayerID)
{
	// Don't do anything if the struct wasn't changed
	if (UniquePlayerID.ExactlyEquals(NewUniquePlayerID))
	{
		return;
	}

	UniquePlayerID = NewUniquePlayerID;
	OnUniquePlayerIdInitializedOrChanged();
}

ECharacterRole AEscapeChroniclesPlayerState::GetCharacterRole() const
{
	if (AbilitySystemComponent->HasMatchingGameplayTag(EscapeChroniclesGameplayTags::Role_Prisoner))
	{
		// Make sure the character doesn't have other roles
#if DO_ENSURE
		ensureAlways(!AbilitySystemComponent->HasMatchingGameplayTag(EscapeChroniclesGameplayTags::Role_Guard));
		ensureAlways(!AbilitySystemComponent->HasMatchingGameplayTag(EscapeChroniclesGameplayTags::Role_Medic));
#endif

		return ECharacterRole::Prisoner;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(EscapeChroniclesGameplayTags::Role_Guard))
	{
		// Make sure the character doesn't have other roles (we already checked for the prisoner role)
#if DO_ENSURE
		ensureAlways(!AbilitySystemComponent->HasMatchingGameplayTag(EscapeChroniclesGameplayTags::Role_Medic));
#endif

		return ECharacterRole::Guard;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(EscapeChroniclesGameplayTags::Role_Medic))
	{
		return ECharacterRole::Medic;
	}

	return ECharacterRole::None;
}