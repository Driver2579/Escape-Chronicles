// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "EscapeChroniclesGameInstance.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/**
	 * This should be called when the player wants to host a new game.
	 * @param HostingPlayerNetID Local player that is hosting the game.
	 * @param OnCreateSessionCompleteDelegate Delegate that is going to be called when the session is created or failed
	 * to be created.
	 * @param OnStartSessionCompleteDelegate Delegate that is going to be called when the session is started or failed
	 * to be started, but only after the session was created successfully.
	 * @remark For clients to join the session, they have to accept the invitation from the host.
	 */
	void StartHostSession(const FUniqueNetId& HostingPlayerNetID,
		const FOnCreateSessionCompleteDelegate& OnCreateSessionCompleteDelegate,
		const FOnStartSessionCompleteDelegate& OnStartSessionCompleteDelegate);

	/**
	 * Calls ServerTravel on the server using the TSoftObjectPtr of the level.
	 * @param Level Level you want to open.
	 * @param bListenServer Whether you want to be a listen server on opened level or not.
	 * @param Options Additional options for the final level URL.
	 * @return Whether the server travel was successful or not.
	 * @remark It is recommended to call this function only after you call StartHostSession and receive a success.
	 */
	bool ServerTravelByLevelSoftObjectPtr(const TSoftObjectPtr<UWorld>& Level, const bool bListenServer = true,
		const FString& Options = FString()) const;

	/**
	 * This should be called when the hosting player wants to exit the game. This function is going to call KickPlayer
	 * for every remote client and destroy this session.
	 * @param OnDestroySessionCompleteDelegate Delegate that is going to be called when the session is destroyed or
	 * failed to be destroyed.
	 * @param bServerTravelToMainMenu If true, then once the session is destroyed, if it was destroyed successfully, the
	 * ServerTravelByLevelSoftObjectPtr function is going to be called with the MainMenuLevel passed.
	 */
	void DestroyHostSession(
		const FOnDestroySessionCompleteDelegate& OnDestroySessionCompleteDelegate =
			FOnDestroySessionCompleteDelegate(),
		const bool bServerTravelToMainMenu = false);

	// Checks if the given player is a remote client
	static bool CanKickPlayer(const APlayerController* PlayerController)
	{
#if DO_CHECK
		check(IsValid(PlayerController));
#endif

		return !PlayerController->IsLocalController() && PlayerController->GetNetMode() != NM_DedicatedServer;
	}

	/**
	 * Travels the given player to the main menu. Should be a remote client.
	 * @remark You MUST call DestroySession once this player gets into the main menu.
	 * @remark You should call CanKickPlayer before calling this function.
	 */
	void KickPlayer(APlayerController* PlayerController) const
	{
#if DO_CHECK
		check(IsValid(PlayerController));
#endif

#if DO_ENSURE
		// Make sure we can kick the player
		ensureAlways(CanKickPlayer(PlayerController));
#endif

		PlayerController->ClientTravel(GetLevelURLFromSoftObjectPtr(MainMenuLevel), TRAVEL_Absolute);
	}

	/**
	 * This MUST be called after the main menu is opened to ensure clients break their connection to the server.
	 * @param OnDestroySessionCompleteDelegate A delegate that is going to be called when the session is destroyed or
	 * failed to be destroyed.
	 * @param bServerTravelToMainMenu If true, then after the session is destroyed, the ServerTravelByLevelSoftObjectPtr
	 * function is going to be called with the MainMenuLevel passed.
	 */
	void DestroySession(
		const FOnDestroySessionCompleteDelegate& OnDestroySessionCompleteDelegate =
			FOnDestroySessionCompleteDelegate(),
		const bool bServerTravelToMainMenu = false);

protected:
	// Called when the session is created after the StartHostSession function is called
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	// Called when the session is started after OnCreateSessionComplete was called with success
	virtual void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	 * Called when the player accepts an invitation to join a session. Joins the session if the invitation was
	 * successful.
	 * @remark If the invitation was accepted by the player already hosting the game, then his session is going to be
	 * terminated.
	 */
	virtual void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId,
		FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	/**
	 * Called when the session is destroyed.
	 * @param SessionName The name of the session.
	 * @param bWasSuccessful Whether the session was destroyed successfully or not.
	 * @param bServerTravelToMainMenu If true, then if the session was successfully destroyed, the
	 * ServerTravelByLevelSoftObjectPtr will be called with the MainMenuLevel passed.
	 */
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful, const bool bServerTravelToMainMenu);

private:
	// The maximum players that can play together in one session
	UPROPERTY(EditDefaultsOnly, Category="Sessions")
	uint8 MaxPlayersPerSession = 4;

	TArray<FDelegateHandle> OnCreateSessionCompleteDelegateHandles;
	TArray<FDelegateHandle> OnStartSessionCompleteDelegateHandles;

	void ClearOnStartSessionCompleteDelegateHandles(const IOnlineSessionPtr SessionInterface);

	// The main menu level that is going to be opened for the client when he's kicked from the server and for the host when he 
	UPROPERTY(EditDefaultsOnly, Category="Sessions")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	// Converts the given TSoftObjectPtr and options to a level URL that can be used for ServerTravel or ClientTravel
	static FString GetLevelURLFromSoftObjectPtr(const TSoftObjectPtr<UWorld>& Level, const FString& Options = FString())
	{
#if DO_ENSURE
		ensureAlways(!Level.IsNull());
#endif

		return Level.GetLongPackageName() + Options;
	}

	TArray<FDelegateHandle> OnDestroySessionCompleteDelegateHandles;
};