// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CHATPlayerController.h"

#include "UI/CHATChatInput.h"
#include "Kismet/KismetSystemLibrary.h"

void ACHATPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}


	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UCHATChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();

			PrintChatMessageString(ChatMessageString);
		}
	}
}

void ACHATPlayerController::SetChatMessageString(const FString& InChatMessageSting)
{
	ChatMessageString = InChatMessageSting;

	PrintChatMessageString(ChatMessageString);
}

void ACHATPlayerController::PrintChatMessageString(const FString& InChatMessageSting)
{
	UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);
}
