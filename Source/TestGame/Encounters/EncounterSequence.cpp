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
        CompleteSequence();
        return;
    }

    Encounter->OnEncounterCompleted.AddDynamic(
        this,
        &AEncounterSequence::HandleEncounterCompleted
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