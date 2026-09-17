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

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[GAME STATE] LevelState=%s | "
            "Authority=TRUE"
        ),
        *UEnum::GetValueAsString(LevelState)
    );

    OnLevelStateChanged.Broadcast(
        LevelState
    );
}

void ATestGameGameState::SetCurrentEncounterIndex(
    int32 NewIndex)
{
    if (!HasAuthority())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[GAME STATE] SetCurrentEncounterIndex rejected | "
                "Authority=FALSE"
            )
        );

        return;
    }

    if (CurrentEncounterIndex == NewIndex)
    {
        return;
    }

    CurrentEncounterIndex = NewIndex;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[GAME STATE] EncounterIndex=%d | "
            "Authority=TRUE"
        ),
        CurrentEncounterIndex
    );

    OnEncounterIndexChanged.Broadcast(
        CurrentEncounterIndex
    );
}

void ATestGameGameState::OnRep_LevelState()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[GAME STATE] LevelState replicated | "
            "State=%s | Authority=%s"
        ),
        *UEnum::GetValueAsString(LevelState),
        HasAuthority()
        ? TEXT("TRUE")
        : TEXT("FALSE")
    );

    OnLevelStateChanged.Broadcast(
        LevelState
    );
}

void ATestGameGameState::
OnRep_CurrentEncounterIndex()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[GAME STATE] EncounterIndex replicated | "
            "Index=%d | Authority=%s"
        ),
        CurrentEncounterIndex,
        HasAuthority()
        ? TEXT("TRUE")
        : TEXT("FALSE")
    );

    OnEncounterIndexChanged.Broadcast(
        CurrentEncounterIndex
    );
}