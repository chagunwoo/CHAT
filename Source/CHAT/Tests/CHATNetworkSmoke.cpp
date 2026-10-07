// Opt-in, editor-only integration test. Exercises real owning-client RPCs and replication.
#if WITH_EDITOR
#include "Player/CHATPlayerController.h"
#include "Player/CHATPlayerState.h"
#include "Game/CHATGameState.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

struct FCHATSmokeState
{
    FTimerHandle Timer;
    int32 Step = 0;
    int32 RoundId = -1;
    int32 ResetTicks = 0;
    bool bSawResult = false;
    bool bFailed = false;
    bool bCaptured = false;
};

void CHATStartNetworkSmoke(ACHATPlayerController* PC)
{
    FString Role, Scenario, CapturePath;
    if (!FParse::Value(FCommandLine::Get(), TEXT("CHATSmokeRole="), Role)) return;
    FParse::Value(FCommandLine::Get(), TEXT("CHATSmokeScenario="), Scenario);
    FParse::Value(FCommandLine::Get(), TEXT("CHATCapture="), CapturePath);
    const TSharedRef<FCHATSmokeState> State = MakeShared<FCHATSmokeState>();
    const TWeakObjectPtr<ACHATPlayerController> WeakPC(PC);
    PC->GetWorldTimerManager().SetTimer(State->Timer, [WeakPC, State, Role, Scenario, CapturePath]()
    {
        if (!WeakPC.IsValid()) return;
        ACHATPlayerController* Controller = WeakPC.Get();
        ACHATGameState* GS = Controller->GetWorld()->GetGameState<ACHATGameState>();
        ACHATPlayerState* PS = Controller->GetPlayerState<ACHATPlayerState>();
        if (!GS || !PS) return;
        const FCHATRoundState& Round = GS->GetRound();
        auto Verify = [&](bool Condition, const TCHAR* Label)
        {
            UE_LOG(LogTemp, Display, TEXT("CHAT_SMOKE_%s %s: %s"), Condition ? TEXT("CHECK") : TEXT("FAIL"), *Role, Label);
            State->bFailed |= !Condition;
        };
        if (State->RoundId < 0)
        {
            if (Round.Phase != ECHATRoundPhase::Playing || !PS->IsRoundParticipant()) return;
            State->RoundId = Round.RoundId;
        }
        if (Round.Phase == ECHATRoundPhase::Result && !State->bSawResult)
        {
            State->bSawResult = true;
            Verify(Round.Announcement.Contains(Scenario == TEXT("Win") ? TEXT("승리") : TEXT("무승부")), TEXT("Correct result replicated"));
            Verify(PS->GetAttemptsUsed() == (Role == TEXT("A") || Scenario == TEXT("Draw") ? 3 : 1), TEXT("Final attempts replicated"));
            // Both clients try to send again during the result phase. No extra attempt or second winner is allowed.
            Controller->SetChatMessageString(TEXT("386"));
            return;
        }
        if (State->bSawResult && Round.Phase == ECHATRoundPhase::Result && !CapturePath.IsEmpty() && !State->bCaptured)
        {
            State->bCaptured = true;
            FScreenshotRequest::RequestScreenshot(CapturePath, true, false);
        }
        if (State->bSawResult && Round.Phase == ECHATRoundPhase::Playing && Round.RoundId > State->RoundId)
        {
            if (++State->ResetTicks < 2) return; // Allow independent PlayerState replication to arrive.
            Verify(PS->GetAttemptsUsed() == 0, TEXT("New round resets local attempts"));
            Verify(GS->PlayerArray.Num() == 2, TEXT("Two remote players present"));
            for (const APlayerState* Player : GS->PlayerArray)
                if (const ACHATPlayerState* Other = Cast<ACHATPlayerState>(Player))
                    Verify(Other->GetAttemptsUsed() == 0 && Other->IsRoundParticipant(), TEXT("All player states reset"));
            const FString Transcript = FString::Join(Controller->GetMessages(), TEXT("\n"));
            Verify(Transcript.Contains(TEXT("SmokeA")) && Transcript.Contains(TEXT("SmokeB")), TEXT("Bidirectional multicast chat received"));
            UE_LOG(LogTemp, Display, TEXT("CHAT_SMOKE_%s %s %s"), State->bFailed ? TEXT("FAILED") : TEXT("PASS"), *Scenario, *Role);
            Controller->GetWorldTimerManager().ClearTimer(State->Timer);
            return;
        }
        if (State->bSawResult || Round.Phase != ECHATRoundPhase::Playing) return;
        const int32 Step = State->Step++;
        if (Step == 0) Controller->SetChatMessageString(TEXT("/say Smoke") + Role);
        if (Role == TEXT("A"))
        {
            if (Step == 1) Controller->SetChatMessageString(TEXT("abc"));
            if (Step == 2) Controller->SetChatMessageString(TEXT("112"));
            if (Step == 3) Controller->SetChatMessageString(TEXT("012"));
            if (Step == 4) Controller->SetChatMessageString(TEXT("12"));
            if (Step == 5)
            {
                Verify(PS->GetAttemptsUsed() == 0, TEXT("Invalid inputs and chat cost no attempts"));
                Controller->ServerSubmitMessage(TEXT("386"), State->RoundId - 1);
            }
            if (Step == 6)
            {
                Verify(PS->GetAttemptsUsed() == 0, TEXT("Stale round RPC rejected"));
                Controller->SetChatMessageString(TEXT("123"));
            }
            if (Step == 7) Controller->SetChatMessageString(TEXT("127"));
            if (Step == 8) Controller->SetChatMessageString(TEXT("167"));
            if (Step == 9) Controller->SetChatMessageString(TEXT("386"));
            if (Step == 10) Verify(PS->GetAttemptsUsed() == 3 && Round.Phase == ECHATRoundPhase::Playing, TEXT("Fourth guess rejected even when correct"));
        }
        else
        {
            if (Step == 10) Verify(PS->GetAttemptsUsed() == 0, TEXT("Attempts independent between players"));
            if (Step == 12) Controller->SetChatMessageString(Scenario == TEXT("Win") ? TEXT("386") : TEXT("123"));
            if (Scenario == TEXT("Draw") && Step == 13) Controller->SetChatMessageString(TEXT("127"));
            if (Scenario == TEXT("Draw") && Step == 14) Controller->SetChatMessageString(TEXT("167"));
        }
    }, 1.0f, true);
}
#endif
