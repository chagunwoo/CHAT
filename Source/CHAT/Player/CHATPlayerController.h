// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CHATPlayerController.generated.h"

class UCHATChatInput;
/**
 * 
 */
UCLASS()
class CHAT_API ACHATPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void SetChatMessageString(const FString& InChatMessageSting);

	void PrintChatMessageString(const FString& InChatMessageSting);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCHATChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UCHATChatInput> ChatInputWidgetInstance;
	
	FString ChatMessageString;
};
