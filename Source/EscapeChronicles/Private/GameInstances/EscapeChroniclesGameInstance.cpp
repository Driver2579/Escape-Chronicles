// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstances/EscapeChroniclesGameInstance.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineSessionInterface.h"

void UEscapeChroniclesGameInstance::Init()
{
	Super::Init();

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (!ensureAlways(OnlineSubsystem))
	{
		return;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	// Listen for a player accepting an invitation to join a session
	if (ensureAlways(SessionInterface.IsValid()))
	{
		SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &ThisClass::OnSessionUserInviteAccepted));
	}
}

void UEscapeChroniclesGameInstance::StartHostSession(const FUniqueNetId& HostingPlayerNetID,
	const FOnCreateSessionCompleteDelegate& OnCreateSessionCompleteDelegate)
{
	const FName& SessionName = NAME_GameSession;

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (!ensureAlways(OnlineSubsystem))
	{
		OnCreateSessionCompleteDelegate.Execute(SessionName, false);

		return;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	if (!ensureAlways(SessionInterface.IsValid()))
	{
		OnCreateSessionCompleteDelegate.Execute(SessionName, false);

		return;
	}

	// Construct the SessionSettings
	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPrivateConnections = MaxPlayersPerSession;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = true;
	SessionSettings.bUseLobbiesIfAvailable = true;

	// Subscribe our OnCreateSessionComplete function to the delegate
	const FDelegateHandle InternalDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));

	// Subscribe the passed delegate to the delegate, LOL
	const FDelegateHandle ExternalDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		OnCreateSessionCompleteDelegate);

	// Remember the delegate handles to clear them later
	OnCreateSessionCompleteDelegateHandles.Add(InternalDelegateHandle);
	OnCreateSessionCompleteDelegateHandles.Add(ExternalDelegateHandle);

	// Create the session for the passed player and with our constructed settings
	SessionInterface->CreateSession(HostingPlayerNetID, SessionName, SessionSettings);
}

void UEscapeChroniclesGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (!ensureAlways(OnlineSubsystem))
	{
		return;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	if (!ensureAlways(SessionInterface.IsValid()))
	{
		return;
	}

	// Clear all delegates
	for (FDelegateHandle& DelegateHandle : OnCreateSessionCompleteDelegateHandles)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(DelegateHandle);
	}

	// Forget about the delegate handles
	OnCreateSessionCompleteDelegateHandles.Empty();
}

bool UEscapeChroniclesGameInstance::ServerTravelByLevelSoftObjectPtr(const TSoftObjectPtr<UWorld>& Level,
	const bool bListenServer, const FString& Options) const
{
#if DO_ENSURE
	ensureAlways(!Level.IsNull());
#endif

	// Convert the TSoftObjectPtr of the given level to a level URL
	FString LevelURL = GetLevelURLFromSoftObjectPtr(Level, Options);

	// Add the "listen" option if we should be a listen server
	if (bListenServer)
	{
		LevelURL += TEXT("?listen");
	}

	// Call the ServerTravel with our constructed LevelURL
	return GetWorld()->ServerTravel(LevelURL, true);
}

void UEscapeChroniclesGameInstance::DestroyHostSession(
	const FOnDestroySessionCompleteDelegate& OnDestroySessionCompleteDelegate, const bool bServerTravelToMainMenu)
{
	const FName& SessionName = NAME_GameSession;

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (!ensureAlways(OnlineSubsystem))
	{
		OnDestroySessionCompleteDelegate.ExecuteIfBound(SessionName, false);

		return;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	if (!ensureAlways(SessionInterface.IsValid()))
	{
		OnDestroySessionCompleteDelegate.ExecuteIfBound(SessionName, false);

		return;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();

	// Kick all remote players from the game
	if (ensureAlways(IsValid(GameState)))
	{
#if DO_ENSURE
		// Make sure we call this function only on a server
		ensureAlways(GameState->HasAuthority());
#endif

		for (const APlayerState* PlayerState : GameState->PlayerArray)
		{
			APlayerController* PlayerController = PlayerState->GetPlayerController();

			// Kick the player if his PlayerController is valid and he can be kicked
			if (IsValid(PlayerController) && CanKickPlayer(PlayerController))
			{
				KickPlayer(PlayerController);
			}
		}
	}

	// Destroy the session. This will also travel us to the main menu if bServerTravelToMainMenu is set to true.
	DestroySession(OnDestroySessionCompleteDelegate, bServerTravelToMainMenu);
}

void UEscapeChroniclesGameInstance::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId,
	FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful)
	{
		return;
	}

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (!ensureAlways(OnlineSubsystem))
	{
		return;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	if (!ensureAlways(SessionInterface.IsValid()))
	{
		return;
	}

	// Get GameState to check if we are the server
	const AGameStateBase* GameState = GetWorld()->GetGameState();

	/**
	 * We might already have an active session, which isn't a problem because JoinSession will automatically handle
	 * switching the sessions, but if we are the server and hosting a game, then we need to kick all the clients first.
	 * To do that correctly, we have to call the DestroyHostSession function. If we have no active session, then it
	 * won't take any effect.
	 */
	if (ensureAlways(IsValid(GameState)) && GameState->HasAuthority())
	{
		DestroyHostSession(FOnDestroySessionCompleteDelegate(), false);
	}

	// Join the session from the invitation and assert if we fail
	ensureAlways(SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult));
}

void UEscapeChroniclesGameInstance::DestroySession(
	const FOnDestroySessionCompleteDelegate& OnDestroySessionCompleteDelegate, const bool bServerTravelToMainMenu)
{
	const FName& SessionName = NAME_GameSession;

	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (!ensureAlways(OnlineSubsystem))
	{
		OnDestroySessionCompleteDelegate.ExecuteIfBound(SessionName, false);

		return;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	if (!ensureAlways(SessionInterface.IsValid()))
	{
		OnDestroySessionCompleteDelegate.ExecuteIfBound(SessionName, false);

		return;
	}

	// Subscribe our OnDestroySessionComplete function to the delegate
	const FDelegateHandle InternalDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete,
			bServerTravelToMainMenu));

	// Subscribe the passed delegate to the delegate, LOL
	const FDelegateHandle ExternalDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		OnDestroySessionCompleteDelegate);

	// Remember the delegate handles to clear them later
	OnDestroySessionCompleteDelegateHandles.Add(InternalDelegateHandle);
	OnDestroySessionCompleteDelegateHandles.Add(ExternalDelegateHandle);
}

void UEscapeChroniclesGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful,
	const bool bServerTravelToMainMenu)
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (ensureAlways(OnlineSubsystem))
	{
		const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (ensureAlways(SessionInterface.IsValid()))
		{
			// Clear all delegates
			for (FDelegateHandle& DelegateHandle : OnDestroySessionCompleteDelegateHandles)
			{
				SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DelegateHandle);
			}

			// Forget about the delegate handles
			OnDestroySessionCompleteDelegateHandles.Empty();
		}
	}

	// If the session was destroyed successfully, then travel to the main menu if it was requested
	if (bWasSuccessful && bServerTravelToMainMenu)
	{
		// We don't want to be a ListenServer in the main menu, and we want to assert if the travel was failed
		ensureAlwaysMsgf(ServerTravelByLevelSoftObjectPtr(MainMenuLevel, false),
			TEXT("Failed for server to travel to main menu!"));
	}
}