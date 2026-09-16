#include "TestGamePlayerState.h"

ATestGamePlayerState::ATestGamePlayerState()
{
}

FString ATestGamePlayerState::GetDebugPlayerName() const
{
    const FString CurrentPlayerName = GetPlayerName();

    if (!CurrentPlayerName.IsEmpty())
    {
        return CurrentPlayerName;
    }

    return FString::Printf(
        TEXT("Player %d"),
        GetPlayerId()
    );
}

FString ATestGamePlayerState::GetDebugPlayerLabel() const
{
    return FString::Printf(
        TEXT("%s [ID=%d]"),
        *GetDebugPlayerName(),
        GetPlayerId()
    );
}