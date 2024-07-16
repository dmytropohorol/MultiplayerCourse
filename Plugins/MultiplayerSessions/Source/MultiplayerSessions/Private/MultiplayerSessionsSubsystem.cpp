// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSessionSettings.h"

DEFINE_LOG_CATEGORY(LogMultiplayerSession);

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	CancelFindSessionsCompleteDelegate(FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnCancelFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete)),
	ReadFriendsListCompleteDelegate(FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnReadFriendsListComplete))
{
	IsValidSessionInterface();
	IsValidFriendsInterface();
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

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!IsValidSessionInterface()) {
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) {
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

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

//@ToDo create auto create session if not already avaiable one
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

	//Sending session invite, session must be created on player who is sending invite. Using SessionInterface function, because Friends
	//Interface SendInvite() is implemented only on EOS and EOSPlus
	if (SessionInterface->SendSessionInviteToFriend(Player->GetControllerId(), NAME_GameSession, *FriendUniqueNetId)) {
		MultiplayerOnSesionInviteSentComplete.Broadcast(true); return; 
	}
	else {
		UE_LOG(LogMultiplayerSession, Warning, TEXT("SessionInterface->SendSessionInviteToFriend returned false, and didnt send invite in UMultiplayerSessionsSubsystem::SendSessionInviteToFriend"));
		MultiplayerOnSesionInviteSentComplete.Broadcast(false); return; 
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
		UE_LOG(LogTemp, Warning, TEXT("Friends Interface is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }
	if (!FriendUniqueNetId.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("Friend Unique Net ID is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }
	if (!PlayerController) {
		UE_LOG(LogTemp, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::GetFriend")); return TSharedPtr<FOnlineFriend>(); }

	return FriendsInterface->GetFriend(Player->GetControllerId(), *FriendUniqueNetId.Get(), EFriendsLists::ToString(EFriendsLists::Default));
}

bool UMultiplayerSessionsSubsystem::IsAFriend(APlayerController* PlayerController, const FUniqueNetIdPtr UniqueNetId)
{
	if (!IsValidFriendsInterface()) {
		UE_LOG(LogTemp, Warning, TEXT("Friends Interface is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false; }
	if (!UniqueNetId.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("Unique Net ID is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false; }
	if (!PlayerController) {
		UE_LOG(LogTemp, Warning, TEXT("Player Controller is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false; }

	ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerController->Player);

	if (!Player) {
		UE_LOG(LogTemp, Warning, TEXT("Local Player is not valid in UMultiplayerSessionsSubsystem::IsAFriend")); return false;
	}
	return FriendsInterface->IsFriend(Player->GetControllerId(), *UniqueNetId.Get(), EFriendsLists::ToString(EFriendsLists::Default));;
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
	UE_LOG(LogTemp, Warning, TEXT("Session invite accepted successfuly"))
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

void UMultiplayerSessionsSubsystem::OnInviteSendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
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
