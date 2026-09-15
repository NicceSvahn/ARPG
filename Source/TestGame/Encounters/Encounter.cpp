#include "Encounter.h"

#include "../Spawners/EnemySpawner.h"
#include "../Characters/EnemyCharacter.h"

AEncounter::AEncounter()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEncounter::BeginPlay()
{
    Super::BeginPlay();

    StartEncounter();
}

void AEncounter::StartEncounter()
{
    if (EncounterState != EEncounterState::Inactive)
    {
        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("ENCOUNTER STARTED: %s"),
        *GetName()
    );

    EncounterState = EEncounterState::Active;
    EncounterEnemies.Reset();

    for (AEnemySpawner* Spawner : EnemySpawners)
    {
        if (!Spawner)
        {
            continue;
        }

        const TArray<AEnemyCharacter*> SpawnedEnemies =
            Spawner->SpawnEnemies();

        for (AEnemyCharacter* Enemy : SpawnedEnemies)
        {
            RegisterEnemy(Enemy);
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("ENCOUNTER: Tracking %d enemies"),
        EncounterEnemies.Num()
    );


    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Encounter started with %d tracked enemies"),
        EncounterEnemies.Num()
    );

    CheckEncounterComplete();
}

void AEncounter::RegisterEnemy(AActor* Enemy)
{
    if (!Enemy)
    {
        return;
    }

    EncounterEnemies.Add(Enemy);

    Enemy->OnDestroyed.AddDynamic(
        this,
        &AEncounter::HandleEnemyDestroyed
    );
}

void AEncounter::HandleEnemyDestroyed(
    AActor* DestroyedActor)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("HandleEnemyDestroyed CALLED")
    );

    EncounterEnemies.Remove(DestroyedActor);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("ENCOUNTER: %s destroyed - %d enemies remaining"),
        *GetNameSafe(DestroyedActor),
        EncounterEnemies.Num()
    );

    CheckEncounterComplete();
}

void AEncounter::CheckEncounterComplete()
{
    if (EncounterState !=
        EEncounterState::Active)
    {
        return;
    }

    EncounterEnemies.RemoveAll(
        [](const TObjectPtr<AActor>& Enemy)
        {
            return !IsValid(Enemy);
        }
    );

    if (!EncounterEnemies.IsEmpty())
    {
        return;
    }

    EncounterState =
        EEncounterState::Completed;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "Encounter Complete: %s"
        ),
        *GetName()
    );

    OnEncounterCompleted.Broadcast();
}

int32 AEncounter::GetAliveEnemyCount() const
{
    int32 AliveCount = 0;

    for (const AActor* Enemy
        : EncounterEnemies)
    {
        if (IsValid(Enemy))
        {
            AliveCount++;
        }
    }

    return AliveCount;
}