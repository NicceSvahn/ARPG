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

    DOREPLIFETIME(
        ATestGameGameState,
        EnemyKillProgress
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

void ATestGameGameState::SetEnemyKillProgress(
    int32 Total,
    int32 Dead
)
{
    if (!HasAuthority())
    {
        return;
    }

    EnemyKillProgress.Total =
        FMath::Max(0, Total);

    EnemyKillProgress.Dead =
        FMath::Clamp(
            Dead,
            0,
            EnemyKillProgress.Total
        );

    OnEnemyKillProgressChanged.Broadcast();
}

int32 ATestGameGameState::GetRequiredEnemyKills() const
{
    return FMath::CeilToInt(
        EnemyKillProgress.Total * 0.8f
    );
}

float ATestGameGameState::GetEnemyProgressToGoal() const
{
    const int32 Required =
        GetRequiredEnemyKills();

    if (Required <= 0)
    {
        return 0.0f;
    }

    return FMath::Clamp(
        static_cast<float>(EnemyKillProgress.Dead) /
        Required,
        0.0f,
        1.0f
    );
}

void ATestGameGameState::OnRep_EnemyKillProgress()
{
    OnEnemyKillProgressChanged.Broadcast();
}