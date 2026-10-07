#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CHATPlayerState.generated.h"

UCLASS()
class CHAT_API ACHATPlayerState : public APlayerState
{
    GENERATED_BODY()
public:
    static constexpr int32 MaxAttempts = 3;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UFUNCTION(BlueprintPure, Category="CHAT") int32 GetAttemptsUsed() const { return AttemptsUsed; }
    UFUNCTION(BlueprintPure, Category="CHAT") int32 GetMaxAttempts() const { return MaxAttempts; }
    UFUNCTION(BlueprintPure, Category="CHAT") int32 GetRemainingAttempts() const { return MaxAttempts - AttemptsUsed; }
    UFUNCTION(BlueprintPure, Category="CHAT") bool IsRoundParticipant() const { return bRoundParticipant; }
    void ResetForRound(bool bParticipating);
    bool ConsumeAttempt();
private:
    UPROPERTY(Replicated) int32 AttemptsUsed = 0;
    UPROPERTY(Replicated) bool bRoundParticipant = false;
};
