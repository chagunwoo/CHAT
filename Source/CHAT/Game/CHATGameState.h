#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CHATGameState.generated.h"

UENUM(BlueprintType)
enum class ECHATRoundPhase : uint8 { Waiting, Playing, Result };

USTRUCT(BlueprintType)
struct CHAT_API FCHATRoundState
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) int32 RoundId = 0;
    UPROPERTY(BlueprintReadOnly) ECHATRoundPhase Phase = ECHATRoundPhase::Waiting;
    UPROPERTY(BlueprintReadOnly) FString Announcement = TEXT("플레이어 2명 이상 접속하면 시작합니다.");
    UPROPERTY(BlueprintReadOnly) double ResetAtServerTime = 0.0;
};

UCLASS()
class CHAT_API ACHATGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    const FCHATRoundState& GetRound() const { return Round; }
    void SetRound(const FCHATRoundState& NewRound);
    UFUNCTION(NetMulticast, Reliable) void MulticastChat(const FString& Message);
private:
    UPROPERTY(Replicated) FCHATRoundState Round;
};
