// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Button.h"
#include "MenuWidget.h"
#include "MultiplayerSessionsSubsystem.h"

void UMenuWidget::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

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

}

bool UMenuWidget::Initialize()
{
	if (!Super::Initialize()){
		return false;
	}

	if (HostButton) {
		HostButton->OnClicked.AddDynamic(this, &UMenuWidget::HostButtonClicked);
	}
	if (JoinButton) {
		JoinButton->OnClicked.AddDynamic(this, &UMenuWidget::JoinButtonClicked);
	}


	return true;
}

void UMenuWidget::HostButtonClicked()
{
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->CreateSession(3, FString("FreeForAll"));
	}
}

void UMenuWidget::JoinButtonClicked()
{
}
