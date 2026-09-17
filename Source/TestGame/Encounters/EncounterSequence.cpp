#include "EncounterSequence.h"

#include "Encounter.h"
#include "TimerManager.h"
#include "../Game/TestGameGameState.h"

AEncounterSequence::AEncounterSequence()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEncounterSequence::StartSequence()
{
    if (!HasAuthority())
    {
        return;
    }

    if (ATestGameGameState* GameState =
        GetWorld()->GetGameState<ATestGameGameState>())
    {
        GameState->OnLevelStateChanged.AddUniqueDynamic(
            this,
            &AEncounterSequence::HandleLevelStateChanged
        );
    }

    if (bSequenceActive)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[ENCOUNTER SEQUENCE] Started | "
            "Sequence=%s | Authority=TRUE"
        ),
        *GetNameSafe(this)
    );

    bSequenceActive = true;
    CurrentEncounterIndex = 0;

    if (ATestGameGameState* GameState =
        GetWorld()->GetGameState<ATestGameGameState>())
    {
        GameState->SetCurrentEncounterIndex(
            CurrentEncounterIndex
        );

        GameState->SetLevelState(
            ELevelState::EncounterActive
        );
    }

    StartCurrentEncounter();
}

void AEncounterSequence::StartCurrentEncounter()
{

    if (!HasAuthority())
    {
        return;
    }

    if (!Encounters.IsValidIndex(CurrentEncounterIndex))
    {
        CompleteSequence();
        return;
    }

    AEncounter* Encounter =
        Encounters[CurrentEncounterIndex];

    if (!Encounter)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "EncounterSequence %s has invalid encounter at index %d"
            ),
            *GetName(),
            CurrentEncounterIndex
        );

        CompleteSequence();
        return;
    }

    Encounter->OnEncounterCompleted.AddDynamic(
        this,
        &AEncounterSequence::HandleEncounterCompleted
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Starting Encounter %d"),
        CurrentEncounterIndex + 1
    );

    if (ATestGameGameState* GameState =
        GetWorld()->GetGameState<ATestGameGameState>())
    {
        GameState->SetCurrentEncounterIndex(
            CurrentEncounterIndex
        );

        GameState->SetLevelState(
            ELevelState::EncounterActive
        );
    }

    Encounter->StartEncounter();
}

void AEncounterSequence::HandleEncounterCompleted()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bSequenceActive)
    {
        return;
    }

    AEncounter* CompletedEncounter =
        Encounters.IsValidIndex(CurrentEncounterIndex)
        ? Encounters[CurrentEncounterIndex]
        : nullptr;

    if (CompletedEncounter)
    {
        CompletedEncounter->OnEncounterCompleted.RemoveDynamic(
            this,
            &AEncounterSequence::HandleEncounterCompleted
        );
    }

    CurrentEncounterIndex++;

    if (!Encounters.IsValidIndex(CurrentEncounterIndex))
    {
        CompleteSequence();
        return;
    }

    GetWorldTimerManager().SetTimer(
        EncounterDelayTimer,
        this,
        &AEncounterSequence::StartCurrentEncounter,
        DelayBetweenEncounters,
        false
    );

    if (ATestGameGameState* GameState =
        GetWorld()->GetGameState<ATestGameGameState>())
    {
        GameState->SetLevelState(
            ELevelState::Intermission
        );
    }
}

void AEncounterSequence::CompleteSequence()
{
    if (!bSequenceActive)
    {
        return;
    }

    bSequenceActive = false;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Encounter Sequence Complete: %s"),
        *GetName()
    );

    if (ATestGameGameState* GameState =
        GetWorld()->GetGameState<ATestGameGameState>())
    {
        GameState->SetLevelState(
            ELevelState::Completed
        );
    }

    OnSequenceCompleted.Broadcast();
}

int32 AEncounterSequence::GetCurrentEncounterNumber() const
{
    if (!bSequenceActive)
    {
        return 0;
    }

    return CurrentEncounterIndex + 1;
}

void AEncounterSequence::StopSequence()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bSequenceActive)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[ENCOUNTER SEQUENCE] Stopped | "
            "Sequence=%s"
        ),
        *GetNameSafe(this)
    );

    bSequenceActive = false;

    GetWorldTimerManager().ClearTimer(
        EncounterDelayTimer
    );

    AEncounter* CurrentEncounter =
        Encounters.IsValidIndex(CurrentEncounterIndex)
        ? Encounters[CurrentEncounterIndex]
        : nullptr;

    if (CurrentEncounter)
    {
        CurrentEncounter->OnEncounterCompleted.RemoveDynamic(
            this,
            &AEncounterSequence::HandleEncounterCompleted
        );
    }
}

void AEncounterSequence::HandleLevelStateChanged(
    ELevelState NewState)
{
    if (!HasAuthority())
    {
        return;
    }

    if (NewState != ELevelState::Failed)
    {
        return;
    }

    StopSequence();
}