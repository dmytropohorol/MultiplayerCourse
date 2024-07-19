// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineAchievementsInterface.h"

DEFINE_LOG_CATEGORY(LogMultiplayerSession);

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete)),
	CancelFindSessionsCompleteDelegate(FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCancelFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnStartSessionComplete)),
	SessionInviteAcceptedDelegate(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnSessionUserInviteAccepted)),
	SessionInviteReceivedDelegate(FOnSessionInviteReceivedDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnSessionInviteReceived)),
	ReadFriendsListCompleteDelegate(FOnReadFriendsListComplete::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnReadFriendsListComplete))
{
	IsValidSessionInterface();
	IsValidFriendsInterface();

	SessionInviteReceivedDelegateHandle = SessionInterface->AddOnSessionInviteReceivedDelegate_Handle(SessionInviteReceivedDelegate);
}

bool UMultiplayerSessionsSubsystem::IsValidSessionInterface()
{
	if (!SessionInterface)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			SessionInterface = Subsystem->GetSessionInterface();

		}
	}
	return SessionInterface.IsValid();
}

bool UMultiplayerSessionsSubsystem::IsValidFriendsInterface()
{
	if (!FriendsInterface)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			FriendsInterface = Subsystem->GetFriendsInterface();

		}
	}
	return FriendsInterface.IsValid();
}

bool UMultiplayerSessionsSubsystem::IsValidExternalUIInterface()
{
	if (!ExternalUIInterface)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			ExternalUIInterface = Subsystem->GetExternalUIInterface();

		}
	}
	return ExternalUIInterface.IsValid();
}

bool UMultiplayerSessionsSubsystem::IsValidAchievementsInterface()
{
	if (!AchievementsInterface)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			AchievementsInterface = Subsystem->GetAchievementsInterface();

		}
	}
	return AchievementsInterface.IsValid();
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!IsValidSessionInterface()) {
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) {
		DestroySession();
	}

	//Store the delegate in a FDelegateHandle so we can remove it later from the delegate list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings)) {
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!IsValidSessionInterface()) {
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = false;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef())) {
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!IsValidSessionInterface()) {
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult)) {
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if(!IsValidSessionInterface()) {
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if(!SessionInterface->DestroySession(NAME_GameSession)) {
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!IsValidSessionInterface()) {
		MultiplayerOnStartSessionComplete.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->StartSession(NAME_GameSession)) {
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);

		MultiplayerOnStartSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::SendSessionInviteToFriend(APlayerController* PlayerController, const FUniqueNetIdPtr FriendUniqueNetId)
{
	//Checking whether the input data is valid
	if (!IsValidSessionInterface()) { 
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Session Interface is not valid in UMultiplayerSessionsSubsystem::SendSessionInviteToFriend")); MultiplayerOnSesionInviteSentComplete.Broadcast(false); return; }
	if (!FriendUniqueNetId.IsValid()) { 
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Friend Unique Net ID is not valid in UMultiplayerSessionsSubsystem::SendSessionInviteToFriend")); MultiplayerOnSesionInviteSentComplete.Broadcast(false); return; }
	if (!PlayerController) { 
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::SendSessionInviteToFriend")); MultiplayerOnSesionInviteSentComplete.Broadcast(false); return; }

	//Creating and checking a local player
	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);	
	if (!Player) { 
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::SendSessionInviteToFriend")); MultiplayerOnSesionInviteSentComplete.Broadcast(false); return; }

	//Creating session if didnt create before. SendSessionInvite wont work if host who sends invite dont have session avaiable
	if (SessionInterface->GetNamedSession(NAME_GameSession) == nullptr) {
		CreateSession();
	}

	SessionInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegate);

	//Sending session invite, session must be created on player who is sending invite. Using SessionInterface function, because Friends
	//Interface SendInvite() is implemented only on EOS and EOSPlus
	if (SessionInterface->SendSessionInviteToFriend(Player->GetControllerId(), NAME_GameSession, *FriendUniqueNetId)) {
		MultiplayerOnSesionInviteSentComplete.Broadcast(true); return; 
	}
	else {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("SessionInterface->SendSessionInviteToFriend returned false, and didnt send invite in UMultiplayerSessionsSubsystem::SendSessionInviteToFriend"));
		MultiplayerOnSesionInviteSentComplete.Broadcast(false); 
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegateHandle); return;
	}
}

void UMultiplayerSessionsSubsystem::GetFriendsList(APlayerController* PlayerController)
{
	if (!IsValidFriendsInterface()) {
		UE_LOG(LogTemp, Warning, TEXT("Friends Interface is not valid in UMultiplayerSessionsSubsystem::GetFriendList"));
		MultiplayerOnGetFriendsListComplete.Broadcast(false, TArray<TSharedRef<FOnlineFriend>>()); return; }
	if (!PlayerController) {
		UE_LOG(LogTemp, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::GetFriendList"));
		MultiplayerOnGetFriendsListComplete.Broadcast(false, TArray<TSharedRef<FOnlineFriend>>()); return; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::GetFriendList"));
		MultiplayerOnGetFriendsListComplete.Broadcast(false, TArray<TSharedRef<FOnlineFriend>>()); return;
	}

	if (!FriendsInterface->ReadFriendsList(Player->GetControllerId(), EFriendsLists::ToString((EFriendsLists::Default)), ReadFriendsListCompleteDelegate)) {
		MultiplayerOnGetFriendsListComplete.Broadcast(false, TArray<TSharedRef<FOnlineFriend>>());
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("FriendsInterface->ReadFriendsList is failed")));
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("FriendsInterface->ReadFriendsList is started")));
	}
}

TSharedPtr<FOnlineFriend> UMultiplayerSessionsSubsystem::GetFriend(APlayerController* PlayerController, const FUniqueNetIdPtr FriendUniqueNetId)
{
	if (!IsValidFriendsInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Friends Interface is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }
	if (!FriendUniqueNetId.IsValid()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Friend Unique Net ID is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }
	if (!PlayerController) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }

	return FriendsInterface->GetFriend(Player->GetControllerId(), *FriendUniqueNetId.Get(), EFriendsLists::ToString(EFriendsLists::Default));
}

bool UMultiplayerSessionsSubsystem::IsAFriend(APlayerController* PlayerController, const FUniqueNetIdPtr UniqueNetId)
{
	if (!IsValidFriendsInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Friends Interface is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false; }
	if (!UniqueNetId.IsValid()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Unique Net ID is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false; }
	if (!PlayerController) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false;
	}
	return FriendsInterface->IsFriend(Player->GetControllerId(), *UniqueNetId.Get(), EFriendsLists::ToString(EFriendsLists::Default));;
}

bool UMultiplayerSessionsSubsystem::ServerTravel(UObject* WorldContextObject, const FString& InURL, bool bAbsolute, bool bShouldSkipGameNotify)
{
	if (!WorldContextObject) {
		UE_LOG(LogTemp, Warning, TEXT("WorldContextObject is not valid in UMultiplayerSessionsSubsystem::ServerTravel"));
		return false;
	}

	//using a context object to get the world
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (World) {
		return World->ServerTravel(InURL, bAbsolute, bShouldSkipGameNotify);
	}
	UE_LOG(LogTemp, Warning, TEXT("World is not valid in UMultiplayerSessionsSubsystem::ServerTravel"));
	return false;
}

void UMultiplayerSessionsSubsystem::ShowProfileUI(const FUniqueNetIdPtr PlayerViewingProfile, const FUniqueNetIdPtr PlayerToViewProfileOf)
{
	if (!IsValidExternalUIInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("ExternalUI Interface is not valid in UMultiplayerSessionsSubsystem::ShowProfileUI")); return; }

	ExternalUIInterface->ShowProfileUI(*PlayerViewingProfile.Get(), *PlayerToViewProfileOf.Get(), NULL);
}

void UMultiplayerSessionsSubsystem::ShowInviteUI(APlayerController* PlayerController)
{
	if (!IsValidExternalUIInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("ExternalUI Interface is not valid in UMultiplayerSessionsSubsystem::ShowInviteUI")); return; }
	if (!PlayerController) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::ShowInviteUI")); return; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::ShowInviteUI")); return; }

	ExternalUIInterface->ShowInviteUI(Player->GetControllerId(), NAME_GameSession);
}

void UMultiplayerSessionsSubsystem::ShowFriendsUI(APlayerController* PlayerController)
{
	if (!IsValidExternalUIInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("ExternalUI Interface is not valid in UMultiplayerSessionsSubsystem::ShowFriendsUI")); return; }
	if (!PlayerController) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::ShowFriendsUI")); return; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::ShowFriendsUI")); return; }

	ExternalUIInterface->ShowFriendsUI(Player->GetControllerId());
}

void UMultiplayerSessionsSubsystem::ShowAchievementsUI(APlayerController* PlayerController)
{
	if (!IsValidExternalUIInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("ExternalUI Interface is not valid in UMultiplayerSessionsSubsystem::ShowAchievementsUI")); return; }
	if (!PlayerController) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::ShowAchievementsUI")); return; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::ShowAchievementsUI")); return; }

	ExternalUIInterface->ShowAchievementsUI(Player->GetControllerId());
}

void UMultiplayerSessionsSubsystem::ShowWebURLUI(FString URLToShow, TArray<FString>& AllowedDomains, bool bEmbedded, bool bShowBackground, bool bShowCloseButton, int32 OffsetX, int32 OffsetY, int32 SizeX, int32 SizeY)
{
	if (!IsValidExternalUIInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("ExternalUI Interface is not valid in UMultiplayerSessionsSubsystem::ShowWebURLUI")); return; }

	URLToShow = URLToShow.Replace(TEXT("http://"), TEXT(""));
	URLToShow = URLToShow.Replace(TEXT("https://"), TEXT(""));

	FShowWebUrlParams Params;
	Params.AllowedDomains = AllowedDomains;
	Params.bEmbedded = bEmbedded;
	Params.bShowBackground = bShowBackground;
	Params.bShowCloseButton = bShowCloseButton;
	Params.OffsetX = OffsetX;
	Params.OffsetY = OffsetY;
	Params.SizeX = SizeX;
	Params.SizeY = SizeY;

	ExternalUIInterface->ShowWebURL(URLToShow, Params);
}

void UMultiplayerSessionsSubsystem::ShowStoreUI(APlayerController* PlayerController, const FShowStoreParams& ShowParams)
{
	if (!IsValidExternalUIInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("ExternalUI Interface is not valid in UMultiplayerSessionsSubsystem::ShowStoreUI")); return; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::ShowStoreUI")); return; }

	ExternalUIInterface->ShowStoreUI(Player->GetControllerId(), ShowParams);
}

void UMultiplayerSessionsSubsystem::ShowSendMessageToUserUI(APlayerController* PlayerController, const FUniqueNetId& Recipient, const FShowSendMessageParams& ShowParams)
{
	if (!IsValidExternalUIInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("ExternalUI Interface is not valid in UMultiplayerSessionsSubsystem::ShowSendMessageToUserUI")); return; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::ShowSendMessageToUserUI")); return; }

	ExternalUIInterface->ShowSendMessageToUserUI(Player->GetControllerId(), Recipient, ShowParams);
}

void UMultiplayerSessionsSubsystem::ReadAchievements(const FUniqueNetIdPtr PlayerId)
{
	if (!IsValidAchievementsInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Achievements Interface is not valid in UMultiplayerSessionsSubsystem::ReadAchievements")); return; }

	AchievementsInterface->QueryAchievements(*PlayerId);
}

void UMultiplayerSessionsSubsystem::ReadAchievementDescriptions(const FUniqueNetIdPtr PlayerId)
{
	if (!IsValidAchievementsInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Achievements Interface is not valid in UMultiplayerSessionsSubsystem::ReadAchievementDescriptions")); return; }

	AchievementsInterface->QueryAchievementDescriptions(*PlayerId);
}

FOnlineAchievement UMultiplayerSessionsSubsystem::GetAchievement(const FUniqueNetIdPtr PlayerId, FString AchievementId)
{
	if (!IsValidAchievementsInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Achievements Interface is not valid in UMultiplayerSessionsSubsystem::GetAchievement")); return FOnlineAchievement(); }
	FOnlineAchievement AchievementStatus;
	if(AchievementsInterface->GetCachedAchievement(*PlayerId, AchievementId, AchievementStatus) == EOnlineCachedResult::Success) {
		return AchievementStatus;
	}
	else {
		return FOnlineAchievement();
	}
}

FOnlineAchievementDesc UMultiplayerSessionsSubsystem::GetAchievementDescription(FString AchievementId)
{
	if (!IsValidAchievementsInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Achievements Interface is not valid in UMultiplayerSessionsSubsystem::GetAchievementDescription")); return FOnlineAchievementDesc(); }
	FOnlineAchievementDesc AchievementDescriptionStatus;
	if (AchievementsInterface->GetCachedAchievementDescription(AchievementId, AchievementDescriptionStatus) == EOnlineCachedResult::Success) {
		return AchievementDescriptionStatus;
	}
	else {
		return FOnlineAchievementDesc();
	}
}

void UMultiplayerSessionsSubsystem::WriteAchievement(const FUniqueNetIdPtr UniqueNetId, FName StatName, float Value)
{
	if (!IsValidAchievementsInterface()) {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("Achievements Interface is not valid in UMultiplayerSessionsSubsystem::WriteAchievement")); return; }

	FOnlineAchievementsWriteRef WriteObject;
	WriteObject->SetFloatStat(StatName, Value);
	AchievementsInterface->WriteAchievements(*UniqueNetId, WriteObject);
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface) {
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface) {
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	if (LastSessionSearch->SearchResults.Num() <= 0) {
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnCancelFindSessionsComplete(bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface) {
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface) {
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy) {
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName,bool bWasSuccessful)
{
	if (SessionInterface) {
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}
	MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Session invite received successfuly"));
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("OnSessionInviteReceived")));
	MultiplayerOnSessionInviteReceived.Broadcast(UserId, FromId,InviteResult);
}

void UMultiplayerSessionsSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Session invite accepted successfuly"));
	if (SessionInterface) {
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionInviteAcceptedDelegateHandle);
	}
	ServerTravel(this, "/Game/Maps/BasicLevel", false, false);
}

void UMultiplayerSessionsSubsystem::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("UMultiplayerSessionsSubsystem::OnReadFriendsListComplete")));
	if (bWasSuccessful) {
		TArray<TSharedRef<FOnlineFriend>> FriendList;
		if (FriendsInterface->GetFriendsList(LocalUserNum, EFriendsLists::ToString((EFriendsLists::Default)), FriendList)) {
			MultiplayerOnGetFriendsListComplete.Broadcast(true, FriendList);
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("UMultiplayerSessionsSubsystem::OnReadFriendsListComplete successfuly")));
		}
		else {
			MultiplayerOnGetFriendsListComplete.Broadcast(false, TArray<TSharedRef<FOnlineFriend>>());
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("FriendsInterface->GetFriendsList failed")));
		}
	}
	else {
		MultiplayerOnGetFriendsListComplete.Broadcast(false, TArray<TSharedRef<FOnlineFriend>>());
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("UMultiplayerSessionsSubsystem::OnReadFriendsListComplete bWasSuccessful is false")));
	}
}