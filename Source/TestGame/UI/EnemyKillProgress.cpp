#include "EnemyKillProgress.h"

#include "../Game/TestGameGameState.h"

#include "Components/ProgressBar.h"
#include "Engine/World.h"

void UEnemyKillProgress::NativeConstruct()
{
    Super::NativeConstruct();

    UnbindGameState();
    RefreshProgress();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GameStateRetryTimer);

        World->GetTimerManager().SetTimer(
            GameStateRetryTimer,
            this,
            &UEnemyKillProgress::TryBindGameState,
            0.25f,
            true
        );

        TryBindGameState();
    }
}

void UEnemyKillProgress::TryBindGameState()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ATestGameGameState* GameState =
        World->GetGameState<ATestGameGameState>();

    if (!IsValid(GameState))
    {
        return;
    }

    if (BoundGameState.Get() != GameState)
    {
        UnbindGameState();
        BoundGameState = GameState;

        GameState->OnEnemyKillProgressChanged.AddUniqueDynamic(
            this,
            &UEnemyKillProgress::RefreshProgress
        );
    }

    World->GetTimerManager().ClearTimer(GameStateRetryTimer);

    RefreshProgress();
}

void UEnemyKillProgress::RefreshProgress()
{
    if (!IsValid(KillProgressBar))
    {
        return;
    }

    const ATestGameGameState* GameState = BoundGameState.Get();
    const float Progress = GameState
        ? GameState->GetEnemyProgressToGoal()
        : 0.0f;

    KillProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
}

void UEnemyKillProgress::UnbindGameState()
{
    if (ATestGameGameState* GameState = BoundGameState.Get())
    {
        GameState->OnEnemyKillProgressChanged.RemoveDynamic(
            this,
            &UEnemyKillProgress::RefreshProgress
        );
    }

    BoundGameState.Reset();
}

void UEnemyKillProgress::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GameStateRetryTimer);
    }

    UnbindGameState();
    Super::NativeDestruct();
}
