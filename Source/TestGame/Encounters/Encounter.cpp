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
    EncounterEnemies.Remove(DestroyedActor);

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

    EncounterState = EEncounterState::Completed;

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