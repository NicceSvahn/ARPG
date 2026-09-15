#include "TestGameGameState.h"

ATestGameGameState::ATestGameGameState()
{
}

void ATestGameGameState::SetLevelState(
    ELevelState NewState)
{
    if (LevelState == NewState)
    {
        return;
    }

    LevelState = NewState;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("LEVEL STATE: %s"),
        *UEnum::GetValueAsString(LevelState)
    );

    OnLevelStateChanged.Broadcast(LevelState);
}

void ATestGameGameState::SetCurrentEncounterIndex(
    int32 NewIndex)
{
    if (CurrentEncounterIndex == NewIndex)
    {
        return;
    }

    CurrentEncounterIndex = NewIndex;

    OnEncounterIndexChanged.Broadcast(
        CurrentEncounterIndex
    );
}