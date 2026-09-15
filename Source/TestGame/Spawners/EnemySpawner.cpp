#pragma once

#include "EnemySpawner.h"
#include "../Characters/EnemyCharacter.h"

#include "NavigationSystem.h"
#include "Engine/World.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

TArray<AEnemyCharacter*> AEnemySpawner::SpawnEnemies()
{
    TArray<AEnemyCharacter*> NewlySpawnedEnemies;

    UWorld* World = GetWorld();

    if (!World)
    {
        return NewlySpawnedEnemies;
    }

    for (const FEnemySpawnEntry& Entry : SpawnEntries)
    {
        if (!Entry.EnemyClass)
        {
            continue;
        }

        for (int32 i = 0; i < Entry.Count; ++i)
        {
            const FVector SpawnLocation =
                GetRandomSpawnLocation();

            if (SpawnLocation == FVector::ZeroVector)
            {
                continue;
            }

            const FRotator SpawnRotation =
                GetActorRotation();

            FActorSpawnParameters SpawnParams;

            SpawnParams.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::
                AdjustIfPossibleButAlwaysSpawn;

            AEnemyCharacter* SpawnedEnemy =
                World->SpawnActor<AEnemyCharacter>(
                    Entry.EnemyClass,
                    SpawnLocation,
                    SpawnRotation,
                    SpawnParams
                );

            if (!SpawnedEnemy)
            {
                continue;
            }

            SpawnedEnemies.Add(SpawnedEnemy);
            NewlySpawnedEnemies.Add(SpawnedEnemy);
        }
    }

    return NewlySpawnedEnemies;
}

FVector AEnemySpawner::GetRandomSpawnLocation() const
{
    UWorld* World = GetWorld();

    if (!World) 
    {
        return GetActorLocation();
    }

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!NavSystem)
    {
        return GetActorLocation();
    }

    FNavLocation NavLocation;

    const bool bFoundLocation = NavSystem->GetRandomReachablePointInRadius(GetActorLocation(), SpawnRadius, NavLocation);

    if (bFoundLocation)
    {
        return NavLocation.Location;
    }
    else
    {
        return FVector::ZeroVector;
    }
}