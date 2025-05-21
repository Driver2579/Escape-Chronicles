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
	const FOnCreateSessionCompleteDelegate& OnCreateSessionCompleteDelegate,
	const FOnStartSessionCompleteDelegate& OnStartSessionCompleteDelegate)
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
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;

	// Subscribe our OnCreateSessionComplete function to the delegate
	const FDelegateHandle InternalOnCreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));

	// Subscribe the passed OnCreateSessionCompleteDelegate to the delegate
	const FDelegateHandle ExternalOnCreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

	// Remember the delegate handles to clear them later
	OnCreateSessionCompleteDelegateHandles.Add(InternalOnCreateSessionCompleteDelegateHandle);
	OnCreateSessionCompleteDelegateHandles.Add(ExternalOnCreateSessionCompleteDelegateHandle);

	// Subscribe our OnStartSessionComplete function to the delegate
	const FDelegateHandle InternalStartSessionDelegateHandle =
		SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
			FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete));

	// Subscribe the passed OnStartSessionCompleteDelegate to the delegate
	const FDelegateHandle ExternalStartSessionDelegateHandle =
		SessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

	// Remember the delegate handles to clear them later
	OnStartSessionCompleteDelegateHandles.Add(InternalStartSessionDelegateHandle);
	OnStartSessionCompleteDelegateHandles.Add(ExternalStartSessionDelegateHandle);

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

	// Start the session if it was successfully created
	if (bWasSuccessful)
	{
		SessionInterface->StartSession(SessionName);
	}
	// Otherwise, clear OnStartSessionComplete delegates
	else
	{
		ClearOnStartSessionCompleteDelegateHandles(SessionInterface);
	}
}

void UEscapeChroniclesGameInstance::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());

	if (!ensureAlways(OnlineSubsystem))
	{
		return;
	}

	const IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();

	// Clear the delegates once the session has started or failed to be started
	if (ensureAlways(SessionInterface.IsValid()))
	{
		ClearOnStartSessionCompleteDelegateHandles(SessionInterface);
	}
}

// ReSharper disable once CppPassValueParameterByConstReference
void UEscapeChroniclesGameInstance::ClearOnStartSessionCompleteDelegateHandles(const IOnlineSessionPtr SessionInterface)
{
#if DO_CHECK
	check(SessionInterface.IsValid());
#endif

	// Clear all delegates
	for (FDelegateHandle& DelegateHandle : OnStartSessionCompleteDelegateHandles)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(DelegateHandle);
	}

	// Forget about the delegate handles
	OnStartSessionCompleteDelegateHandles.Empty();
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

	// Finally, destroy the session
	SessionInterface->DestroySession(SessionName);
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

	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	/**
	 * We might already have an active session, so we need to destroy it first. Thse will have no effect if we don't
	 * have any active sessions. If we are the server, then call a host version of session destruction function.
	 */
	if (GameState->HasAuthority())
	{
		DestroyHostSession();
	}
	// Otherwise, destroy the session as a client
	else
	{
		DestroySession();
	}

	// Subscribe our OnJoinSessionComplete function to the delegate
	const FDelegateHandle DelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

	// Remember the delegate handle to clear it later
	OnJoinSessionCompleteDelegateHandles.Add(DelegateHandle);

	// Join the session from the invitation
	SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult);
}

void UEscapeChroniclesGameInstance::OnJoinSessionComplete(FName SessionName,
	EOnJoinSessionCompleteResult::Type OnJoinSessionCompleteResult)
{
	// Broadcast the delegate before we have any chance to fail in this function
	OnJoinSession.Broadcast(SessionName, OnJoinSessionCompleteResult);

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
	for (FDelegateHandle& DelegateHandle : OnJoinSessionCompleteDelegateHandles)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(DelegateHandle);
	}

	// Forget about the delegate handles
	OnJoinSessionCompleteDelegateHandles.Empty();

	// We don't need to do anything else if we failed to join the session
	if (OnJoinSessionCompleteResult != EOnJoinSessionCompleteResult::Success)
	{
		return;
	}

	// Get the string we're going to use for ClientTravel
	FString ConnectString;
	const bool bGetConnectStringResult = SessionInterface->GetResolvedConnectString(SessionName, ConnectString);

	// Assert if we failed to get the ConnectString and destroy the session because we already connected to it
	if (!ensureAlways(bGetConnectStringResult))
	{
		DestroySession();

		return;
	}

	// Travel all local clients to the level where the host of the session is
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const TWeakObjectPtr<APlayerController> PlayerController = *It;

		if (PlayerController.IsValid() && PlayerController->IsLocalController())
		{
			PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
		}
	}
}