#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CHATPlayerController.generated.h"
class UCHATChatInput;

UCLASS()
class CHAT_API ACHATPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
    void SetChatMessageString(const FString& Message);
    void PrintChatMessageString(const FString& Message);
    UFUNCTION(Server, Reliable) void ServerSubmitMessage(const FString& Message, int32 RoundId);
    UFUNCTION(Client, Reliable) void ClientNotice(const FString& Message);
    UFUNCTION(Exec) void CHATSubmit(const FString& Message);
    const TArray<FString>& GetMessages() const { return Messages; }
    int32 GetMessageRevision() const { return MessageRevision; }
private:
    UPROPERTY(Transient) TObjectPtr<UCHATChatInput> ChatInputWidgetInstance;
    TArray<FString> Messages;
    int32 MessageRevision = 0;
    double LastServerMessageTime = -1.0;
};
