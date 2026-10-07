#include "Player/CHATPlayerController.h"
#include "Game/CHATGameModeBase.h"
#include "Game/CHATGameState.h"
#include "UI/CHATChatInput.h"
#include "Engine/World.h"

void ACHATPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController() || GetNetMode() == NM_DedicatedServer) return;
    ChatInputWidgetInstance = CreateWidget<UCHATChatInput>(this, UCHATChatInput::StaticClass());
    if (ChatInputWidgetInstance)
    {
        ChatInputWidgetInstance->AddToViewport();
        bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(ChatInputWidgetInstance->TakeWidget());
        SetInputMode(InputMode);
    }
}
void ACHATPlayerController::SetChatMessageString(const FString& Message)
{
    if (Message.Len() > 200)
    {
        PrintChatMessageString(TEXT("1~200자 이내로 입력해주세요."));
        return;
    }
    const ACHATGameState* GS = GetWorld()->GetGameState<ACHATGameState>();
    ServerSubmitMessage(Message, GS ? GS->GetRound().RoundId : -1);
}
void ACHATPlayerController::ServerSubmitMessage_Implementation(const FString& Message, int32 RoundId)
{
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastServerMessageTime < 0.2) return;
    LastServerMessageTime = Now;
    if (ACHATGameModeBase* GM = GetWorld()->GetAuthGameMode<ACHATGameModeBase>()) GM->HandleMessage(this, Message, RoundId);
}
void ACHATPlayerController::ClientNotice_Implementation(const FString& Message)
{
    PrintChatMessageString(Message);
}
void ACHATPlayerController::PrintChatMessageString(const FString& Message)
{
    Messages.Add(Message);
    if (Messages.Num() > 100) Messages.RemoveAt(0);
    ++MessageRevision;
    UE_LOG(LogTemp, Log, TEXT("CHAT received: %s"), *Message);
}
void ACHATPlayerController::CHATSubmit(const FString& Message)
{
    SetChatMessageString(Message);
}
