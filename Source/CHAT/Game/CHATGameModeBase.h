#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CHATGameModeBase.generated.h"
class ACHATPlayerController;

UCLASS()
class CHAT_API ACHATGameModeBase : public AGameModeBase
{
    GENERATED_BODY()
public:
    ACHATGameModeBase();
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void HandleMessage(ACHATPlayerController* Sender, const FString& Message, int32 SubmittedRoundId);
    void ResetGame();
private:
    void FinishRound(const FString& Announcement);
    void CheckDraw();
    FString SecretNumbers;
    TArray<TWeakObjectPtr<ACHATPlayerController>> ConnectedPlayers;
    FTimerHandle ResetTimer;
};
