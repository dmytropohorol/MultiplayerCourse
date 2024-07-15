// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Menu.generated.h"

class UButton;
class UVerticalBox;
class UMultiplayerSessionsSubsystem;
/**
 * 
 */
UCLASS()
class MULTIPLAYERCOURSE_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup();

protected:

	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnCreateSession(bool bWasSuccessful);

	void OnGetFriendsList(bool bWasSuccessful, TArray<TSharedRef<FOnlineFriend>> FriendsList);

private:

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ReloadFriendsListButton;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* FriendsListBox;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void ReloadFriendsListButtonClicked();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFriendWidgetItem> FriendWidgetClass;

	// The subsystem designed to handle all online session functionality
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
