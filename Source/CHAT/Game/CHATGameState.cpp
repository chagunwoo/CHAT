#include "Game/CHATGameState.h"
#include "Player/CHATPlayerController.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
void ACHATGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACHATGameState, Round);
}
void ACHATGameState::SetRound(const FCHATRoundState& NewRound)
{
    if (!HasAuthority()) return;
    Round = NewRound;
    ForceNetUpdate();
}
void ACHATGameState::MulticastChat_Implementation(const FString& Message)
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        if (ACHATPlayerController* PC = Cast<ACHATPlayerController>(It->Get()); PC && PC->IsLocalController())
            PC->PrintChatMessageString(Message);
}
