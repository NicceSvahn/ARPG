#include "TestGameGameState.h"

#include "Net/UnrealNetwork.h"

ATestGameGameState::ATestGameGameState()
{
}

void ATestGameGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(
        OutLifetimeProps
    );

    DOREPLIFETIME(
        ATestGameGameState,
        LevelState
    );

    DOREPLIFETIME(
        ATestGameGameState,
        CurrentEncounterIndex
    );
}

void ATestGameGameState::SetLevelState(
    ELevelState NewState)
{
    if (!HasAuthority())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[GAME STATE] SetLevelState rejected | "
                "Authority=FALSE"
            )
        );

        return;
    }

    if (LevelState == NewState)
    {
        return;
    }

    LevelState = NewState;

    OnLevelStateChanged.Broadcast(
        LevelState
    );
}

void ATestGameGameState::SetCurrentEncounterIndex(
    int32 NewIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    if (CurrentEncounterIndex == NewIndex)
    {
        return;
    }

    CurrentEncounterIndex = NewIndex;

    OnEncounterIndexChanged.Broadcast(
        CurrentEncounterIndex
    );
}

void ATestGameGameState::OnRep_LevelState()
{
    OnLevelStateChanged.Broadcast(
        LevelState
    );
}

void ATestGameGameState::
OnRep_CurrentEncounterIndex()
{
    OnEncounterIndexChanged.Broadcast(
        CurrentEncounterIndex
    );
}