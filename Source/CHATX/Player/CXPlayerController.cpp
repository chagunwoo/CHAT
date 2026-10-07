// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CXPlayerController.h"

#include "UI/CXChatInput.h"
#include "Kismet/KismetSystemLibrary.h"

void ACXPlayerController::BeginPlay()
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
		ChatInputWidgetInstance = CreateWidget<UCXChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();

			PrintChatMessageString(ChatMessageString);
		}
	}
}

void ACXPlayerController::SetChatMessageString(const FString& InChatMessageSting)
{
	ChatMessageString = InChatMessageSting;

	PrintChatMessageString(ChatMessageString);
}

void ACXPlayerController::PrintChatMessageString(const FString& InChatMessageSting)
{
	UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);
}
