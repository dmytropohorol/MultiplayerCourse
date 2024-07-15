// Fill out your copyright notice in the Description page of Project Settings.


#include "FriendWidgetItem.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UFriendWidgetItem::WidgetSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->MultiplayerOnSesionInviteSentComplete.AddUObject(this, &UFriendWidgetItem::OnInviteSent);
	}

	FriendName->SetText(FText::FromString(FriendInfo->GetDisplayName()));
}

bool UFriendWidgetItem::Initialize()
{
	if (!Super::Initialize()) {
		return false;
	}

	if (InviteButton) {
		InviteButton->OnClicked.AddDynamic(this, &UFriendWidgetItem::SendInvite);
	}
	return true;
}

void UFriendWidgetItem::NativeConstruct()
{
	Super::NativeConstruct();
}

void UFriendWidgetItem::NativeDestruct()
{
	Super::NativeDestruct();
}

void UFriendWidgetItem::SendInvite()
{
	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController) { 
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("UFriendWidgetItem::SendInvite")));
			MultiplayerSessionsSubsystem->SendSessionInviteToFriend(PlayerController, FriendInfo->GetUserId());
		}
	}
}

void UFriendWidgetItem::OnInviteSent(bool bWasSuccessful)
{
	if (!bWasSuccessful) {
		UE_LOG(LogTemp, Warning, TEXT("Invite failed to send"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Invite successfuly send"));
}
