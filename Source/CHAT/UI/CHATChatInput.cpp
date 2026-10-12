#include "UI/CHATChatInput.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Game/CHATGameState.h"
#include "Game/CHATGameModeBase.h"
#include "Player/CHATPlayerController.h"
#include "Player/CHATPlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"

TSharedRef<SWidget> UCHATChatInput::RebuildWidget()
{
    UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
    Background->SetBrushColor(FLinearColor(0.025f, 0.04f, 0.07f, 1.0f));
    Background->SetPadding(FMargin(36.0f, 24.0f));
    WidgetTree->RootWidget = Background;
    UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>();
    Background->SetContent(Layout);

    auto MakeText = [this, Layout](const FString& Text, int32 Size, FLinearColor Color)
    {
        UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>();
        Block->SetText(FText::FromString(Text));
        FSlateFontInfo Font = Block->GetFont();
        Font.Size = Size;
        Block->SetFont(Font);
        Block->SetColorAndOpacity(FSlateColor(Color));
        Block->SetAutoWrapText(true);
        Layout->AddChildToVerticalBox(Block)->SetPadding(FMargin(0, 0, 0, 12));
        return Block;
    };
    MakeText(TEXT("CHAT  /  NUMBER BASEBALL"), 30, FLinearColor(0.3f, 0.95f, 0.72f));
    MakeText(TEXT("1~9 · 중복 없는 세 자리 · 플레이어당 세 번의 기회"), 16, FLinearColor(0.75f, 0.8f, 0.9f));
    Status = MakeText(TEXT("서버에 연결 중..."), 18, FLinearColor::White);
    Scoreboard = MakeText(TEXT(""), 17, FLinearColor(0.7f, 0.85f, 1.0f));
    Announcement = MakeText(TEXT(""), 24, FLinearColor(1.0f, 0.8f, 0.25f));

#if WITH_EDITOR
    const APlayerController* Owner = GetOwningPlayer();
    if (Owner && Owner->IsLocalController() && Owner->HasAuthority())
    {
        UCheckBox* Toggle = WidgetTree->ConstructWidget<UCheckBox>();
        UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>();
        Label->SetText(FText::FromString(TEXT("시연용 정답 보기 (호스트만)")));
        Toggle->SetContent(Label);
        Toggle->SetIsChecked(bShowDemoAnswer);
        Toggle->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnDemoAnswerChanged);
        Layout->AddChildToVerticalBox(Toggle)->SetPadding(FMargin(0, 0, 0, 8));
        DemoAnswer = MakeText(TEXT(""), 18, FLinearColor(0.3f, 0.95f, 0.72f));
        DemoAnswer->SetVisibility(ESlateVisibility::Collapsed);
    }
#endif

    HistoryScroll = WidgetTree->ConstructWidget<UScrollBox>();
    UVerticalBoxSlot* ScrollSlot = Layout->AddChildToVerticalBox(HistoryScroll);
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ScrollSlot->SetPadding(FMargin(0, 8, 0, 16));
    History = WidgetTree->ConstructWidget<UTextBlock>();
    History->SetAutoWrapText(true);
    FSlateFontInfo HistoryFont = History->GetFont();
    HistoryFont.Size = 18;
    History->SetFont(HistoryFont);
    HistoryScroll->AddChild(History);

    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
    Layout->AddChildToVerticalBox(Row);
    EditableTextBox_ChatInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("EditableTextBox_ChatInput"));
    EditableTextBox_ChatInput->SetHintText(FText::FromString(TEXT("숫자: 123   |   채팅: /say 안녕하세요")));
    EditableTextBox_ChatInput->SetForegroundColor(FLinearColor(0.025f, 0.035f, 0.06f));
    EditableTextBox_ChatInput->WidgetStyle.SetFocusedForegroundColor(FSlateColor(FLinearColor(0.025f, 0.035f, 0.06f)));
    EditableTextBox_ChatInput->WidgetStyle.TextStyle.SetColorAndOpacity(FSlateColor(FLinearColor(0.025f, 0.035f, 0.06f)));
    EditableTextBox_ChatInput->SetClearKeyboardFocusOnCommit(false);
    Row->AddChildToHorizontalBox(EditableTextBox_ChatInput)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    UButton* Send = WidgetTree->ConstructWidget<UButton>();
    Row->AddChildToHorizontalBox(Send)->SetPadding(FMargin(12, 0, 0, 0));
    UTextBlock* SendLabel = WidgetTree->ConstructWidget<UTextBlock>();
    SendLabel->SetText(FText::FromString(TEXT("  전송  ")));
    Send->AddChild(SendLabel);
    Send->OnClicked.AddDynamic(this, &ThisClass::SubmitInput);
    MakeText(TEXT("Enter로 전송 · S: 숫자와 위치 일치 · B: 숫자만 일치 · OUT: 일치 없음"), 14, FLinearColor(0.6f, 0.68f, 0.8f));
    return Super::RebuildWidget();
}
void UCHATChatInput::NativeConstruct()
{
    Super::NativeConstruct();
    if (EditableTextBox_ChatInput)
        EditableTextBox_ChatInput->OnTextCommitted.AddUniqueDynamic(this, &ThisClass::OnChatInputTextCommitted);
    GetWorld()->GetTimerManager().SetTimer(RefreshTimer, this, &ThisClass::RefreshView, 0.1f, true);
    RefreshView();
}
void UCHATChatInput::NativeDestruct()
{
    if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(RefreshTimer);
    if (EditableTextBox_ChatInput)
        EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
    Super::NativeDestruct();
}
void UCHATChatInput::FocusInput()
{
    if (EditableTextBox_ChatInput) EditableTextBox_ChatInput->SetUserFocus(GetOwningPlayer());
}
void UCHATChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter) SubmitInput();
}
void UCHATChatInput::SubmitInput()
{
    ACHATPlayerController* PC = GetOwningPlayer<ACHATPlayerController>();
    if (!PC || !EditableTextBox_ChatInput) return;
    const FString Message = EditableTextBox_ChatInput->GetText().ToString();
    if (Message.IsEmpty()) return;
    PC->SetChatMessageString(Message);
    EditableTextBox_ChatInput->SetText(FText::GetEmpty());
    EditableTextBox_ChatInput->SetKeyboardFocus();
    RefreshView();
}
void UCHATChatInput::RefreshView()
{
    const ACHATPlayerController* PC = GetOwningPlayer<ACHATPlayerController>();
    if (!PC || !Status) return;
    const ACHATGameState* GS = GetWorld()->GetGameState<ACHATGameState>();
    const ACHATPlayerState* LocalPS = PC->GetPlayerState<ACHATPlayerState>();
#if WITH_EDITOR
    if (DemoAnswer)
    {
        const ACHATGameModeBase* GM = GetWorld()->GetAuthGameMode<ACHATGameModeBase>();
        const bool bVisible = bShowDemoAnswer && PC->IsLocalController() && PC->HasAuthority() && GM;
        DemoAnswer->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        DemoAnswer->SetText(bVisible ? FText::FromString(GM->GetDemoAnswer().IsEmpty()
            ? TEXT("[시연용] 플레이어 접속 대기 중")
            : FString::Printf(TEXT("[시연용] 이번 라운드 정답: %s"), *GM->GetDemoAnswer())) : FText::GetEmpty());
    }
#endif
    if (GS)
    {
        const FCHATRoundState& Round = GS->GetRound();
        FString StatusText = FString::Printf(TEXT("ROUND %d"), Round.RoundId);
        if (LocalPS) StatusText += FString::Printf(TEXT("  |  %s  |  내 시도 [%d / %d]"), *LocalPS->GetPlayerName(), LocalPS->GetAttemptsUsed(), LocalPS->GetMaxAttempts());
        Status->SetText(FText::FromString(StatusText));
        FString Banner = Round.Announcement;
        if (Round.Phase == ECHATRoundPhase::Result)
            Banner += FString::Printf(TEXT("\n%d초 후 새 게임을 시작합니다."), FMath::Max(0, FMath::CeilToInt(Round.ResetAtServerTime - GS->GetServerWorldTimeSeconds())));
        else if (Round.Phase == ECHATRoundPhase::Playing && LocalPS)
        {
            if (!LocalPS->IsRoundParticipant()) Banner = TEXT("관전 중 · 다음 라운드부터 참여합니다.");
            else if (LocalPS->GetRemainingAttempts() == 0) Banner = TEXT("기회를 모두 사용했습니다. 다른 플레이어의 결과를 기다립니다.");
        }
        Announcement->SetText(FText::FromString(Banner));
        FString Scores;
        for (const APlayerState* Player : GS->PlayerArray)
        {
            if (const ACHATPlayerState* PS = Cast<ACHATPlayerState>(Player))
                Scores += FString::Printf(TEXT("%s%s [%d / %d]%s\n"),
                    PS == LocalPS ? TEXT("● ") : TEXT("○ "), *PS->GetPlayerName(), PS->GetAttemptsUsed(), PS->GetMaxAttempts(),
                    PS->IsRoundParticipant() ? TEXT("") : TEXT(" (대기)"));
        }
        Scoreboard->SetText(FText::FromString(Scores.TrimEnd()));
    }
    if (LastMessageRevision != PC->GetMessageRevision())
    {
        LastMessageRevision = PC->GetMessageRevision();
        History->SetText(FText::FromString(FString::Join(PC->GetMessages(), TEXT("\n"))));
        HistoryScroll->ScrollToEnd();
    }
}

void UCHATChatInput::OnDemoAnswerChanged(bool bChecked)
{
#if WITH_EDITOR
    const APlayerController* Owner = GetOwningPlayer();
    bShowDemoAnswer = bChecked && Owner && Owner->IsLocalController() && Owner->HasAuthority();
    RefreshView();
#endif
}
