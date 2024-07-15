// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "FriendWidgetItem.h"

void UMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddUObject(this, &UMenu::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnGetFriendsListComplete.AddUObject(this, &UMenu::OnGetFriendsList);
	}
}

bool UMenu::Initialize()
{

	if (!Super::Initialize()) {
		return false;
	}

	if (HostButton) {
		HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
	}
	if (ReloadFriendsListButton) {
		ReloadFriendsListButton->OnClicked.AddDynamic(this, &UMenu::ReloadFriendsListButtonClicked);
	}

	return true;
}

void UMenu::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMenu::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Successfuly created session")));
		}
		UWorld* World = GetWorld();
		if (World) {
			//World->ServerTravel();
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Failed to create session")));
		}
	}
}

void UMenu::OnGetFriendsList(bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>> FriendsList)
{
	if (!bWasSuccessful) {
		UE_LOG(LogTemp, Warning, TEXT("bWasSuccessuful in OnGetFriendsList is false"));
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Successfuly got friends")));

	for (auto Friend : FriendsList) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString(Friend->GetDisplayName()));
		UFriendWidgetItem* FriendWidget = CreateWidget<UFriendWidgetItem>(this, FriendWidgetClass);
		FriendWidget->FriendInfo = Friend;
		FriendsListBox->AddChildToVerticalBox(FriendWidget);
		FriendWidget->WidgetSetup();
	}
}

void UMenu::HostButtonClicked()
{
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->CreateSession(2, TEXT("Default"));
	}
}

void UMenu::ReloadFriendsListButtonClicked()
{
	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) {
			if (MultiplayerSessionsSubsystem) {
				MultiplayerSessionsSubsystem->GetFriendsList(PlayerController);
			}
		}
	}
}
