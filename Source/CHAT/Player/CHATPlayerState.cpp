#include "Player/CHATPlayerState.h"
#include "Net/UnrealNetwork.h"

void ACHATPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACHATPlayerState, AttemptsUsed);
    DOREPLIFETIME(ACHATPlayerState, bRoundParticipant);
}
void ACHATPlayerState::ResetForRound(bool bParticipating)
{
    if (!HasAuthority()) return;
    AttemptsUsed = 0;
    bRoundParticipant = bParticipating;
    ForceNetUpdate();
}
bool ACHATPlayerState::ConsumeAttempt()
{
    if (!HasAuthority() || !bRoundParticipant || AttemptsUsed >= MaxAttempts) return false;
    ++AttemptsUsed;
    ForceNetUpdate();
    return true;
}
