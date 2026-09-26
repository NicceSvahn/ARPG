#include "EncounterManager.h"

#include "../LevelGeneration/LevelGenerator.h"
#include "../LevelGeneration/LevelChunkDefinition.h"
#include "../Characters/EnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"

AEncounterManager::AEncounterManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

void AEncounterManager::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (!GetWorld() ||
        !GetWorld()->IsGameWorld() ||
        !HasAuthority() ||
        !IsValid(LevelGenerator))
    {
        return;
    }

    RandomStream.Initialize(EncounterSeed);

    LevelGenerator->OnGeneratedMapReady.AddUObject(
        this,
        &AEncounterManager::HandleMapReady
    );

    LevelGenerator->OnGeneratedChunksClearing.AddUObject(
        this,
        &AEncounterManager::HandleChunksClearing
    );
}

void AEncounterManager::EndPlay(
    const EEndPlayReason::Type EndPlayReason
)
{
    if (IsValid(LevelGenerator))
    {
        LevelGenerator->OnGeneratedMapReady.RemoveAll(this);
        LevelGenerator->OnGeneratedChunksClearing.RemoveAll(this);
    }

    HandleChunksClearing();
    Super::EndPlay(EndPlayReason);
}

void AEncounterManager::HandleMapReady()
{
    if (!HasAuthority() || bMapPopulated)
    {
        return;
    }

    NavigationRetryCount = 0;

    // Streamed geometry and dynamic navigation need some time
    // after the last chunk becomes visible.
    GetWorldTimerManager().SetTimer(
        NavigationRetryTimer,
        this,
        &AEncounterManager::TryPopulateMap,
        0.5f,
        true
    );

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ENCOUNTER] Map visible; waiting for navigation.")
    );
}

void AEncounterManager::TryPopulateMap()
{
    if (!HasAuthority() ||
        !IsValid(LevelGenerator) ||
        bMapPopulated)
    {
        GetWorldTimerManager().ClearTimer(NavigationRetryTimer);
        return;
    }

    UNavigationSystemV1* NavSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(
            GetWorld()
        );

    if (!NavSystem ||
        NavSystem->IsNavigationBuildInProgress())
    {
        ++NavigationRetryCount;

        if (NavigationRetryCount >= 80)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[ENCOUNTER] Timed out waiting for "
                    "navigation to finish building."
                )
            );

            GetWorldTimerManager().ClearTimer(
                NavigationRetryTimer
            );
        }

        return;
    }

    GetWorldTimerManager().ClearTimer(NavigationRetryTimer);
    bMapPopulated = true;

    for (const FGeneratedChunkInstance& Chunk :
        LevelGenerator->GetChunkInstances())
    {
        if (IsValid(Chunk.Definition) &&
            Chunk.Definition->ChunkType ==
            ELevelChunkType::Combat)
        {
            SpawnInChunk(Chunk, NavSystem);
        }
    }
}

void AEncounterManager::SpawnInChunk(
    const FGeneratedChunkInstance& Chunk,
    UNavigationSystemV1* NavSystem
)
{
    if (!EnemyClass || !NavSystem || !GetWorld())
    {
        return;
    }

    const float ChunkSize =
        LevelGenerator->GetChunkSize();

    const float UsableHalfSize =
        ChunkSize * 0.5f - SpawnInset;

    if (UsableHalfSize <= 0.0f)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ENCOUNTER] SpawnInset is too large.")
        );
        return;
    }

    // Matches the generator's current GridToWorldLocation().
    const FVector ChunkCenter(
        Chunk.GridCoordinate.X * ChunkSize,
        Chunk.GridCoordinate.Y * ChunkSize,
        0.0f
    );

    const AEnemyCharacter* EnemyDefaults =
        EnemyClass->GetDefaultObject<AEnemyCharacter>();

    const UCapsuleComponent* Capsule =
        EnemyDefaults
        ? EnemyDefaults->GetCapsuleComponent()
        : nullptr;

    const float CapsuleHalfHeight =
        Capsule
        ? Capsule->GetScaledCapsuleHalfHeight()
        : 100.0f;

    const int32 MinCount =
        FMath::Max(0, MinEnemiesPerCombatChunk);

    const int32 MaxCount =
        FMath::Max(
            MinCount,
            MaxEnemiesPerCombatChunk
        );

    const int32 DesiredCount =
        RandomStream.RandRange(MinCount, MaxCount);

    int32 SpawnedCount = 0;

    const int32 MaxAttempts =
        FMath::Max(20, DesiredCount * 20);

    for (int32 Attempt = 0;
        Attempt < MaxAttempts &&
        SpawnedCount < DesiredCount;
        ++Attempt)
    {
        const FVector Candidate(
            ChunkCenter.X +
            RandomStream.FRandRange(
                -UsableHalfSize,
                UsableHalfSize
            ),
            ChunkCenter.Y +
            RandomStream.FRandRange(
                -UsableHalfSize,
                UsableHalfSize
            ),
            ChunkCenter.Z
        );

        FNavLocation NavPoint;

        if (!NavSystem->ProjectPointToNavigation(
            Candidate,
            NavPoint,
            FVector(75.0f, 75.0f, 300.0f)
        ))
        {
            continue;
        }

        const FVector NavPosition =
            NavPoint.Location;

        // A navigation projection must remain in this chunk.
        if (FMath::Abs(
            NavPosition.X - ChunkCenter.X
        ) > UsableHalfSize ||
            FMath::Abs(
                NavPosition.Y - ChunkCenter.Y
            ) > UsableHalfSize)
        {
            continue;
        }

        bool bNearPlayer = false;

        for (FConstPlayerControllerIterator It =
            GetWorld()->GetPlayerControllerIterator();
            It;
            ++It)
        {
            const APlayerController* Controller =
                It->Get();

            if (Controller &&
                Controller->GetPawn() &&
                FVector::Dist2D(
                    NavPosition,
                    Controller->GetPawn()->GetActorLocation()
                ) < MinDistanceFromPlayers)
            {
                bNearPlayer = true;
                break;
            }
        }

        if (bNearPlayer)
        {
            continue;
        }

        const FVector SpawnPosition =
            NavPosition +
            FVector(
                0.0f,
                0.0f,
                CapsuleHalfHeight + 2.0f
            );

        const FRotator SpawnRotation(
            0.0f,
            RandomStream.FRandRange(
                -180.0f,
                180.0f
            ),
            0.0f
        );

        FActorSpawnParameters Params;

        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::
            AdjustIfPossibleButDontSpawnIfColliding;

        AEnemyCharacter* Enemy =
            GetWorld()->SpawnActor<AEnemyCharacter>(
                EnemyClass,
                SpawnPosition,
                SpawnRotation,
                Params
            );

        if (IsValid(Enemy))
        {
            SpawnedEnemies.Add(Enemy);
            ++SpawnedCount;
        }
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[ENCOUNTER] Combat chunk (%d,%d): "
            "spawned %d/%d enemies."
        ),
        Chunk.GridCoordinate.X,
        Chunk.GridCoordinate.Y,
        SpawnedCount,
        DesiredCount
    );
}

void AEncounterManager::HandleChunksClearing()
{
    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(
            NavigationRetryTimer
        );
    }

    if (HasAuthority())
    {
        for (const TWeakObjectPtr<AEnemyCharacter>& Enemy :
            SpawnedEnemies)
        {
            if (Enemy.IsValid())
            {
                Enemy->Destroy();
            }
        }
    }

    SpawnedEnemies.Reset();
    NavigationRetryCount = 0;
    bMapPopulated = false;
}