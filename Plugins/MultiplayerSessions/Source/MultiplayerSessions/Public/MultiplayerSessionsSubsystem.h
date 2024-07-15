// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MultiplayerSessionsSubsystem.generated.h"

//Dealing with default session controlling delegates
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool bWasSuccessful);

//Find sessions delegates
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessul);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCancelFindSessionsComplete, bool bWasSuccessul);

//Session invite delegates
DECLARE_MULTICAST_DELEGATE_ThreeParams(FMultiplayerOnSessionInviteReceived, const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionSearchResult& InviteResult);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FMultiplayerOnSessionInviteAccepted, const bool bWasSuccessful, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSesionInviteSentComplete, bool bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnGetFriendsListComplete, bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>> FriendsList);

enum class SteamAvatarSize : uint8
{
	SteamAvatar_INVALID = 0,
	SteamAvatar_Small = 1,
	SteamAvatar_Medium = 2,
	SteamAvatar_Large = 3
};

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	UMultiplayerSessionsSubsystem();

	void CreateSession(int32 NumPublicConnections, FString MatchType);
	//@Task: add specifiers for finding sessions
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	void SendSessionInviteToFriend(APlayerController* PlayerController, const FUniqueNetIdPtr FriendUniqueNetId);
	void GetFriendsList(APlayerController* PlayerController);
	TSharedPtr<FOnlineFriend> GetFriend(APlayerController* PlayerController, const FUniqueNetIdPtr FriendUniqueNetId);
	bool IsAFriend(APlayerController* PlayerController, const FUniqueNetIdPtr UniqueNetId);

	bool ServerTravel(UObject* WorldContextObject, const FString& InURL, bool bAbsolute, bool bShouldSkipGameNotify);

	/*
	UTexture2D* GetSteamFriendAvatar(const FUniqueNetIdPtr UniqueNetId, SteamAvatarSize AvatarSize = SteamAvatarSize::SteamAvatar_Medium);
	bool RequestSteamFriendInfo(const FUniqueNetIdPtr UniqueNetId, bool bRequireNameOnly = false);
	int32 GetSteamFriendGamePlayed(const FUniqueNetIdPtr UniqueNetId);

	void ShowProfileUI(const FUniqueNetIdPtr PlayerViewingProfile, const FUniqueNetIdPtr PlayerToViewProfileOf);
	void ShowInviteUI(APlayerController* PlayerController);
	void ShowWebURLUI(FString URLToShow, TArray<FString>& AllowedDomains, bool bEmbedded = false, bool bShowBackground = false, bool bShowCloseButton = false, int32 OffsetX = 0, int32 OffsetY = 0, int32 SizeX = 0, int32 SizeY = 0);
	void CloseWebURLUI();

	bool KickPlayer(UObject* WorldContextObject, APlayerController* PlayerToKick, FText KickReason);

	bool GetSessionSettings(UObject* WorldContextObject, FOnlineSessionSettings& SessionSettings);
	*/


	//Create and destroy session delegate
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;

	//Join session delegate
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;

	//Start session delegate
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	//Find session and cancel find session delegates
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnCancelFindSessionsComplete MultiplayerOnCancelFindSessionsComplete;

	//Session invites releated delegates
	FMultiplayerOnSessionInviteReceived MultiplayerOnSessionInviteReceived;
	FMultiplayerOnSessionInviteAccepted MultiplayerOnSessionInviteAccepted;
	FMultiplayerOnSesionInviteSentComplete MultiplayerOnSesionInviteSentComplete;

	FMultiplayerOnGetFriendsListComplete MultiplayerOnGetFriendsListComplete;

protected:

	//Callbacks for IOnlineSubsystem delegates
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnCancelFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

private:
	
	bool IsValidSessionInterface();
	bool IsValidFriendsInterface();

	IOnlineFriendsPtr FriendsInterface;
	IOnlineSessionPtr SessionInterface;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;

	//Settings and Results for Create and Find Session
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	
	//Create session
	//Delegate fired when a session create request has completed
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	// Find session
	//Delegate fired when the search for an online session has completed
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	// Cancel find session
	//Delegate fired when the cancellation of a search for an online session has completed
	FOnCancelFindSessionsCompleteDelegate CancelFindSessionsCompleteDelegate;
	FDelegateHandle CancelFindSessionsCompleteDelegateHandle;

	//Join Session
	//Delegate fired when the joining process for an online session has completed
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	//Destroy session
	//Delegate fired when a destroying an online session has completed
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	//Start session
	//Delegate fired when the online session has transitioned to the started state
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	//Session invite received
	//Called when a user receives a session invitation. Allows the game code to decide on accepting the invite.
	//The invite can be accepted by calling JoinSession()
	FOnSessionInviteReceivedDelegate SessionInviteReceivedDelegate;
	FDelegateHandle SessionInviteReceivedDelegateHandle;

	//Session invite accepted
	//Called when a user accepts a session invitation. Allows the game code a chance
	//to clean up any existing state before accepting the invite. 
	//The invite must be accepted by calling JoinSession() after clean up has completed
	FOnSessionUserInviteAcceptedDelegate SessionInviteAcceptedDelegate;
	FDelegateHandle SessionInviteAcceptedDelegateHandle;

	//Read friends list
	//Delegate used when the friends read request has completed
	FOnReadFriendsListComplete ReadFriendsListCompleteDelegate;
	FDelegateHandle ReadFriendsListCompleteDelegateHandle;
};
