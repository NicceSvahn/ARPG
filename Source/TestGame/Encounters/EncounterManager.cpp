#include "EncounterManager.h"


#include "../Spawners/EnemySpawnMarker.h"
#include "../LevelGeneration/LevelGenerator.h"
#include "../LevelGeneration/LevelChunkDefinition.h"
#include "../Characters/EnemyCharacter.h"

#include "Engine/Level.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"

AEncounterManager::AEncounterManager()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = false;
}

void AEncounterManager::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (!HasAuthority() || !IsValid(LevelGenerator))
    {
        return;
    }

    RandomStream.Initialize(EncounterSeed);

    LevelGenerator->OnGeneratedChunkReady.AddUObject(
        this,
        &AEncounterManager::HandleChunkReady
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
        LevelGenerator->OnGeneratedChunkReady.RemoveAll(this);
        LevelGenerator->OnGeneratedChunksClearing.RemoveAll(this);
    }

    HandleChunksClearing();

    Super::EndPlay(EndPlayReason);
}

void AEncounterManager::HandleChunkReady(
    ULevelStreamingDynamic* StreamingLevel,
    ULevelChunkDefinition* Definition,
    FIntPoint GridCoordinate
)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[ENCOUNTER] ChunkReady (%d,%d), type=%d, class=%s"),
        GridCoordinate.X,
        GridCoordinate.Y,
        IsValid(Definition)
        ? static_cast<int32>(Definition->ChunkType)
        : -1,
        *GetNameSafe(EnemyClass.Get())
    );

    if (!HasAuthority() ||
        !IsValid(StreamingLevel) ||
        !IsValid(Definition) ||
        Definition->ChunkType != ELevelChunkType::Combat ||
        !EnemyClass)
    {
        return;
    }

    ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel();
    UWorld* World = GetWorld();

    if (!LoadedLevel || !World)
    {
        return;
    }

    TArray<AEnemySpawnMarker*> Markers;

    //Search this instance's loaded level, not the whole world.
    //Multiple WFC cells can use the same tile asset.
    for (AActor* Actor : LoadedLevel->Actors)
    {
        if (AEnemySpawnMarker* Marker =
            Cast<AEnemySpawnMarker>(Actor))
        {
            Markers.Add(Marker);
        }
    }

    if (Markers.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[ENCOUNTER] Combat chunk (%d,%d) "
                "has no spawn markers."
            ),
            GridCoordinate.X,
            GridCoordinate.Y
        );

        return;
    }

    const int32 MinCount =
        FMath::Max(0, MinEnemiesPerCombatChunk);

    const int32 MaxCount =
        FMath::Max(
            MinCount,
            MaxEnemiesPerCombatChunk
        );

    const int32 DesiredCount =
        RandomStream.RandRange(
            MinCount,
            MaxCount
        );

    //Fisher-Yates shuffle. Afterward, taking the first N
    //markers chooses N distinct locations.
    for (int32 Index = Markers.Num() - 1;
        Index > 0;
        --Index)
    {
        const int32 SwapIndex =
            RandomStream.RandRange(
                0,
                Index
            );

        Markers.Swap(
            Index,
            SwapIndex
        );
    }

    const int32 AttemptCount =
        FMath::Min(
            DesiredCount,
            Markers.Num()
        );

    int32 SuccessfulSpawns = 0;

    for (int32 Index = 0;
        Index < AttemptCount;
        ++Index)
    {
        AEnemySpawnMarker* Marker = Markers[Index];

        if (!IsValid(Marker))
        {
            continue;
        }

        FActorSpawnParameters SpawnParameters;

        SpawnParameters.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::
            AdjustIfPossibleButDontSpawnIfColliding;

        AEnemyCharacter* Enemy =
            World->SpawnActor<AEnemyCharacter>(
                EnemyClass,
                Marker->GetActorLocation(),
                Marker->GetActorRotation(),
                SpawnParameters
            );

        if (!IsValid(Enemy))
        {
            continue;
        }

        SpawnedEnemies.Add(Enemy);
        ++SuccessfulSpawns;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[ENCOUNTER] Combat chunk (%d,%d): "
            "spawned %d enemies from %d markers."
        ),
        GridCoordinate.X,
        GridCoordinate.Y,
        SuccessfulSpawns,
        Markers.Num()
    );
}

void AEncounterManager::HandleChunksClearing()
{
    if (!HasAuthority())
    {
        return;
    }

    for (const TWeakObjectPtr<AEnemyCharacter>& Enemy :
        SpawnedEnemies)
    {
        if (Enemy.IsValid())
        {
            Enemy->Destroy();
        }
    }

    SpawnedEnemies.Reset();
}