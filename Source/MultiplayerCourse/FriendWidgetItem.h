// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSubsystem.h"

#include "FriendWidgetItem.generated.h"

class UButton;
class UTextBlock;
class UMultiplayerSessionsSubsystem;

/**
 * 
 */
UCLASS()
class MULTIPLAYERCOURSE_API UFriendWidgetItem : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void WidgetSetup();

	TSharedPtr<FOnlineFriend> FriendInfo;

protected:

	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:

	UPROPERTY(meta=(BindWidget))
	UButton* InviteButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FriendName;

	UFUNCTION()
	void SendInvite();

	void OnInviteSent(bool bWasSuccessful);

	// The subsystem designed to handle all online session functionality
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
