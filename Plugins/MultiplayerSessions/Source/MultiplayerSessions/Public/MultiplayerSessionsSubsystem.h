// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "MultiplayerSessionsSubsystem.generated.h"

//Dealing with default session controlling delegates
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessul);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCancelFindSessionsComplete, bool bWasSuccessul);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FMultiplayerOnSessionInviteReceived, const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionSearchResult& InviteResult);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FMultiplayerOnSessionInviteAccepted, const bool bWasSuccessful, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnSesionInviteSentComplete, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnGetFriendsListComplete, bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>> FriendsList);

DECLARE_LOG_CATEGORY_EXTERN(LogMultiplayerSession, Log, All);

class FOnlineUserPresence;

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

	//Session Inteface
	void CreateSession(int32 NumPublicConnections = 0, FString MatchType = "Default");
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	//Friends Inteface
	void SendSessionInviteToFriend(APlayerController* PlayerController, const FUniqueNetIdPtr FriendUniqueNetId);
	void GetFriendsList(APlayerController* PlayerController);
	TSharedPtr<FOnlineFriend> GetFriend(APlayerController* PlayerController, const FUniqueNetIdPtr FriendUniqueNetId);
	bool IsAFriend(APlayerController* PlayerController, const FUniqueNetIdPtr UniqueNetId);

	//ExternalUI Interface
	void ShowProfileUI(const FUniqueNetIdPtr PlayerViewingProfile, const FUniqueNetIdPtr PlayerToViewProfileOf);
	void ShowInviteUI(APlayerController* PlayerController);
	void ShowFriendsUI(APlayerController* PlayerController);
	void ShowAchievementsUI(APlayerController* PlayerController);
	void ShowWebURLUI(FString URLToShow, TArray<FString>& AllowedDomains, bool bEmbedded = false, bool bShowBackground = false, bool bShowCloseButton = false, int32 OffsetX = 0, int32 OffsetY = 0, int32 SizeX = 0, int32 SizeY = 0);
	void ShowStoreUI(APlayerController* PlayerController, const FShowStoreParams& ShowParams);
	void ShowSendMessageToUserUI(APlayerController* PlayerController, const FUniqueNetId& Recipient, const FShowSendMessageParams& ShowParams);

	//Achievement Interaface
	void ReadAchievements(const FUniqueNetIdPtr PlayerId);
	void ReadAchievementDescriptions(const FUniqueNetIdPtr PlayerId);
	FOnlineAchievement GetAchievement(const FUniqueNetIdPtr PlayerId, FString AchievementId);
	FOnlineAchievementDesc GetAchievementDescription(FString AchievementId);
	void WriteAchievement(const FUniqueNetIdPtr UniqueNetId, FName StatName, float Value);

	bool ServerTravel(UObject* WorldContextObject, const FString& InURL, bool bAbsolute, bool bShouldSkipGameNotify);

	const FOnlineUserPresence GetPresence()

	/*
	UTexture2D* GetSteamFriendAvatar(const FUniqueNetIdPtr UniqueNetId, SteamAvatarSize AvatarSize = SteamAvatarSize::SteamAvatar_Medium);
	bool RequestSteamFriendInfo(const FUniqueNetIdPtr UniqueNetId, bool bRequireNameOnly = false);
	int32 GetSteamFriendGamePlayed(const FUniqueNetIdPtr UniqueNetId);

	AdvancedSessionsLibrary.h
	bool KickPlayer(UObject* WorldContextObject, APlayerController* PlayerToKick, FText KickReason);
	bool BanPlayer
	void AddOrModifyExtraSettings
	void GetExtraSettings
	void GetSessionState
	bool GetSessionSettings(UObject* WorldContextObject, FOnlineSessionSettings& SessionSettings);
	void IsPlayerInSession
	bool IsValidSession
	void GetSessionID_AsString
	void GetCurrentSessionID_AsString
	void GetCurrentUniqueBuildID
	void GetUniqueBuildID
	void GetUniqueNetID
	void GetUniqueNetIDFromPlayerState
	bool IsValidUniqueNetID
	void UniqueNetIdToString
	void GetPlayerName
	void SetPlayerName
	void GetNumberOfNetworkPlayers
	void GetNetPlayerIndex
	bool HasOnlineSubsystem

	AdvancedIndentityLibrary.h
	ALL FUNCTIONS
	*/

	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnCancelFindSessionsComplete MultiplayerOnCancelFindSessionsComplete;
	FMultiplayerOnSessionInviteReceived MultiplayerOnSessionInviteReceived;
	FMultiplayerOnSessionInviteAccepted MultiplayerOnSessionInviteAccepted;
	FMultiplayerOnSesionInviteSentComplete MultiplayerOnSesionInviteSentComplete;
	FMultiplayerOnGetFriendsListComplete MultiplayerOnGetFriendsListComplete;

protected:

	//Sessions interaface callbacks
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnCancelFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	//Friends interface callbacks
	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	//Achievements interface callbacks

private:
	
	bool IsValidSessionInterface();
	bool IsValidFriendsInterface();
	bool IsValidExternalUIInterface();
	bool IsValidAchievementsInterface();

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	IOnlineFriendsPtr FriendsInterface;
	IOnlineSessionPtr SessionInterface;
	IOnlineExternalUIPtr ExternalUIInterface;
	IOnlineAchievementsPtr AchievementsInterface;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;

	//Delegate fired when a session create request has completed
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	//Delegate fired when the search for an online session has completed
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	//Delegate fired when the cancellation of a search for an online session has completed
	FOnCancelFindSessionsCompleteDelegate CancelFindSessionsCompleteDelegate;
	FDelegateHandle CancelFindSessionsCompleteDelegateHandle;

	//Delegate fired when the joining process for an online session has completed
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	//Delegate fired when a destroying an online session has completed
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	//Delegate fired when the online session has transitioned to the started state
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	//Called when a user accepts a session invitation. Allows the game code a chance
	//to clean up any existing state before accepting the invite.The invite must be
	//accepted by calling JoinSession() after clean up has completed
	FOnSessionUserInviteAcceptedDelegate SessionInviteAcceptedDelegate;
	FDelegateHandle SessionInviteAcceptedDelegateHandle;

	//Called when a user receives a session invitation. Allows the game code to decide
	//on accepting the invite.The invite can be accepted by calling JoinSession()
	FOnSessionInviteReceivedDelegate SessionInviteReceivedDelegate;
	FDelegateHandle SessionInviteReceivedDelegateHandle;

	//Delegate used when the friends read request has completed
	FOnReadFriendsListComplete ReadFriendsListCompleteDelegate;
};
