#include "Game/CHATGameModeBase.h"
#include "Game/CHATGameState.h"
#include "Game/CHATNumberLibrary.h"
#include "Player/CHATPlayerController.h"
#include "Player/CHATPlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY_STATIC(LogCHATGame, Log, All);

ACHATGameModeBase::ACHATGameModeBase()
{
    PlayerControllerClass = ACHATPlayerController::StaticClass();
    PlayerStateClass = ACHATPlayerState::StaticClass();
    GameStateClass = ACHATGameState::StaticClass();
    DefaultPawnClass = nullptr;
}
void ACHATGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    ACHATPlayerController* PC = Cast<ACHATPlayerController>(NewPlayer);
    if (!PC) return;
    ConnectedPlayers.Add(PC);
    if (ACHATPlayerState* PS = PC->GetPlayerState<ACHATPlayerState>())
    {
        PS->ResetForRound(false);
        PS->SetPlayerName(FString::Printf(TEXT("Player %d"), PS->GetPlayerId()));
    }
    ACHATGameState* GS = GetGameState<ACHATGameState>();
    if (GS && GS->GetRound().Phase == ECHATRoundPhase::Waiting) ResetGame();
    else PC->ClientNotice(TEXT("진행 중인 라운드는 관전합니다. 다음 라운드부터 참여합니다."));
}
void ACHATGameModeBase::Logout(AController* Exiting)
{
    ConnectedPlayers.RemoveAll([Exiting](const TWeakObjectPtr<ACHATPlayerController>& PC)
    { return !PC.IsValid() || PC.Get() == Exiting; });
    Super::Logout(Exiting);
    ACHATGameState* GS = GetGameState<ACHATGameState>();
    if (!GS || GS->GetRound().Phase != ECHATRoundPhase::Playing) return;
    if (ConnectedPlayers.Num() < 2)
    {
        FinishRound(TEXT("상대가 퇴장하여 라운드가 취소되었습니다."));
        return;
    }
    CheckDraw();
}
void ACHATGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(ResetTimer);
    Super::EndPlay(EndPlayReason);
}
void ACHATGameModeBase::ResetGame()
{
    if (!HasAuthority()) return;
    GetWorldTimerManager().ClearTimer(ResetTimer);
    ConnectedPlayers.RemoveAll([](const TWeakObjectPtr<ACHATPlayerController>& PC) { return !PC.IsValid(); });
    ACHATGameState* GS = GetGameState<ACHATGameState>();
    if (!GS) return;
    const bool bReady = ConnectedPlayers.Num() >= 2;
    for (const auto& PC : ConnectedPlayers)
        if (ACHATPlayerState* PS = PC->GetPlayerState<ACHATPlayerState>()) PS->ResetForRound(bReady);
    FCHATRoundState Round;
    Round.RoundId = GS->GetRound().RoundId + 1;
    if (bReady)
    {
        SecretNumbers = UCHATNumberLibrary::GenerateRandomNumbers();
#if WITH_EDITOR
        // Deterministic only for explicitly requested editor network tests; absent from packaged games.
        if (FParse::Param(FCommandLine::Get(), TEXT("CHATSmokeServer"))) SecretNumbers = TEXT("386");
#endif
        Round.Phase = ECHATRoundPhase::Playing;
        Round.Announcement = TEXT("게임 시작! 1~9의 서로 다른 숫자 3개를 입력하세요.");
#if !UE_BUILD_SHIPPING
        UE_LOG(LogCHATGame, Log, TEXT("Round %d secret: %s"), Round.RoundId, *SecretNumbers);
#endif
    }
    else SecretNumbers.Reset();
    GS->SetRound(Round);
    GS->MulticastChat(Round.Announcement);
}
void ACHATGameModeBase::HandleMessage(ACHATPlayerController* Sender, const FString& Message, int32 SubmittedRoundId)
{
    if (!HasAuthority() || !IsValid(Sender) || !ConnectedPlayers.Contains(Sender)) return;
    ACHATGameState* GS = GetGameState<ACHATGameState>();
    ACHATPlayerState* PS = Sender->GetPlayerState<ACHATPlayerState>();
    if (!GS || !PS) return;
    if (Message.IsEmpty() || Message.Len() > 200)
    {
        Sender->ClientNotice(TEXT("1~200자 이내로 입력해주세요."));
        return;
    }
    if (Message.StartsWith(TEXT("/say ")))
    {
        FString Chat = Message.Mid(5).TrimStartAndEnd();
        Chat.ReplaceInline(TEXT("\r"), TEXT(" "));
        Chat.ReplaceInline(TEXT("\n"), TEXT(" "));
        Chat.ReplaceInline(TEXT("\t"), TEXT(" "));
        if (Chat.IsEmpty()) Sender->ClientNotice(TEXT("채팅 내용을 입력해주세요."));
        else GS->MulticastChat(FString::Printf(TEXT("%s: %s"), *PS->GetPlayerName(), *Chat));
        return;
    }
    if (GS->GetRound().Phase != ECHATRoundPhase::Playing || SubmittedRoundId != GS->GetRound().RoundId)
    {
        Sender->ClientNotice(TEXT("현재 라운드가 준비되지 않았습니다. 잠시 후 다시 입력하세요."));
        return;
    }
    if (!PS->IsRoundParticipant())
    {
        Sender->ClientNotice(TEXT("다음 라운드부터 참여할 수 있습니다."));
        return;
    }
    if (PS->GetRemainingAttempts() <= 0)
    {
        Sender->ClientNotice(TEXT("3회의 기회를 모두 사용했습니다. 다음 라운드를 기다려주세요."));
        return;
    }
    FString Error;
    if (!UCHATNumberLibrary::ValidateInput(Message, Error))
    {
        Sender->ClientNotice(Error);
        return;
    }
    const FCHATAnswerResult Result = UCHATNumberLibrary::CheckAnswer(SecretNumbers, Message);
    if (!PS->ConsumeAttempt()) return;
    const FString Report = FString::Printf(TEXT("%s > %s : %s  [시도 %d / %d]"),
        *PS->GetPlayerName(), *Message, *Result.ToString(), PS->GetAttemptsUsed(), PS->GetMaxAttempts());
    GS->MulticastChat(Report);
    UE_LOG(LogCHATGame, Log, TEXT("%s"), *Report);
    if (Result.Strikes == 3) FinishRound(FString::Printf(TEXT("%s 승리! 정답: %s"), *PS->GetPlayerName(), *SecretNumbers));
    else CheckDraw();
}
void ACHATGameModeBase::CheckDraw()
{
    int32 Participants = 0;
    for (const auto& PC : ConnectedPlayers)
    {
        const ACHATPlayerState* PS = PC.IsValid() ? PC->GetPlayerState<ACHATPlayerState>() : nullptr;
        if (!PS || !PS->IsRoundParticipant()) continue;
        ++Participants;
        if (PS->GetRemainingAttempts() > 0) return;
    }
    FinishRound(Participants > 0 ? FString::Printf(TEXT("무승부! 정답: %s"), *SecretNumbers) : TEXT("참가자가 없어 라운드를 종료합니다."));
}
void ACHATGameModeBase::FinishRound(const FString& Announcement)
{
    ACHATGameState* GS = GetGameState<ACHATGameState>();
    if (!GS || GS->GetRound().Phase != ECHATRoundPhase::Playing) return;
    FCHATRoundState Round = GS->GetRound();
    Round.Phase = ECHATRoundPhase::Result;
    Round.Announcement = Announcement;
    Round.ResetAtServerTime = GS->GetServerWorldTimeSeconds() + 5.0;
    GS->SetRound(Round);
    GS->MulticastChat(Announcement);
    UE_LOG(LogCHATGame, Log, TEXT("Round ended: %s"), *Announcement);
    GetWorldTimerManager().SetTimer(ResetTimer, this, &ThisClass::ResetGame, 5.0f, false);
}
