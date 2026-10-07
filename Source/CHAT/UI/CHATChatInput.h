#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CHATChatInput.generated.h"
class UEditableTextBox;
class UTextBlock;
class UScrollBox;

UCLASS()
class CHAT_API UCHATChatInput : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    void FocusInput();
protected:
    UFUNCTION() void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    UFUNCTION() void SubmitInput();
    UFUNCTION() void RefreshView();
    UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;
private:
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Status;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Scoreboard;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Announcement;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> History;
    UPROPERTY(Transient) TObjectPtr<UScrollBox> HistoryScroll;
    FTimerHandle RefreshTimer;
    int32 LastMessageRevision = -1;
};
