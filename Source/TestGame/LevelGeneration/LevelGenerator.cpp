#include "LevelGenerator.h"

#include "Engine/LevelStreamingDynamic.h"
#include "Engine/LevelStreaming.h"
#include "WFC/WFCLevelSolver.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "Components/BrushComponent.h"

ALevelGenerator::ALevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ALevelGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (!bGenerateOnBeginPlay)
    {
        return;
    }

    GenerateLevel();

    if (!GeneratedChunks.IsEmpty())
    {
        SpawnGeneratedChunks();
    }
}

void ALevelGenerator::GenerateLevel()
{
    if (!SpawnedChunkLevels.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[WFC] Cannot generate while chunk levels are spawned. "
                "Clear generated chunks first."
            )
        );

        return;
    }

    GeneratedChunks.Reset();

    TArray<ULevelChunkDefinition*> Definitions;
    Definitions.Reserve(WFCTiles.Num());

    for (ULevelChunkDefinition* Definition : WFCTiles)
    {
        if (IsValid(Definition))
        {
            Definitions.Add(Definition);
        }
    }

    if (Definitions.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[WFC] No tile definitions configured.")
        );

        return;
    }

    FWFCLevelSolver Solver;

    if (!Solver.Solve(
        GridWidth,
        GridHeight,
        Definitions,
        GenerationSeed
    ))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC] Generation failed. "
                "Grid=%dx%d Seed=%d"
            ),
            GridWidth,
            GridHeight,
            GenerationSeed
        );

        return;
    }

    GeneratedChunks.Reserve(
        GridWidth * GridHeight
    );

    for (const FWFCCell& Cell : Solver.GetCells())
    {
        if (!Cell.IsCollapsed())
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[WFC] Solved grid contains an unresolved "
                    "Cell(%d,%d)."
                ),
                Cell.Coordinate.X,
                Cell.Coordinate.Y
            );

            GeneratedChunks.Reset();
            return;
        }

        const FWFCState& State =
            Cell.PossibleStates[0];

        if (!IsValid(State.Definition))
        {
            GeneratedChunks.Reset();
            return;
        }

        FGeneratedChunk& GeneratedChunk =
            GeneratedChunks.AddDefaulted_GetRef();

        GeneratedChunk.Definition =
            State.Definition;

        GeneratedChunk.GridCoordinate =
            Cell.Coordinate;

        GeneratedChunk.Rotation =
            State.Rotation;
    }

    if (!ValidateGeneratedChunks())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[WFC] Generated layout failed final validation.")
        );

        GeneratedChunks.Reset();
        return;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[WFC] Generation complete. "
            "%d tiles generated. Grid=%dx%d Seed=%d"
        ),
        GeneratedChunks.Num(),
        GridWidth,
        GridHeight,
        GenerationSeed
    );
}

void ALevelGenerator::SpawnGeneratedChunks()
{
    if (GeneratedChunks.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[WFC] No generated chunks to spawn. "
                "Generate the level first."
            )
        );

        return;
    }

    if (!SpawnedChunkLevels.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[WFC] Chunk levels are already spawned. "
                "Clear them before spawning again."
            )
        );

        return;
    }

    int32 SpawnedCount = 0;

    for (const FGeneratedChunk& Chunk : GeneratedChunks)
    {
        if (!IsValid(Chunk.Definition))
        {
            continue;
        }

        if (Chunk.Definition->ChunkLevel.IsNull())
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "[WFC] Tile %s has no ChunkLevel assigned."
                ),
                *Chunk.Definition->ChunkId.ToString()
            );

            continue;
        }

        const FVector WorldLocation =
            GridToWorldLocation(
                Chunk.GridCoordinate
            );

        const FRotator WorldRotation =
            ChunkRotationToWorldRotation(
                Chunk.Rotation
            );

        bool bLoadSuccess = false;

        ULevelStreamingDynamic* StreamingLevel =
            ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
                this,
                Chunk.Definition->ChunkLevel,
                WorldLocation,
                WorldRotation,
                bLoadSuccess,
                FString(),
                nullptr,
                false
            );

        if (!bLoadSuccess || !IsValid(StreamingLevel))
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[WFC] Failed to spawn %s at Grid(%d,%d)."
                ),
                *Chunk.Definition->ChunkId.ToString(),
                Chunk.GridCoordinate.X,
                Chunk.GridCoordinate.Y
            );

            continue;
        }

        FGeneratedChunkInstance& Instance =
            GeneratedChunkInstances.AddDefaulted_GetRef();

        Instance.StreamingLevel = StreamingLevel;
        Instance.Definition = Chunk.Definition;
        Instance.GridCoordinate = Chunk.GridCoordinate;

        StreamingLevel->OnLevelShown.AddDynamic(
            this,
            &ALevelGenerator::HandleChunkLevelShown
        );

        SpawnedChunkLevels.Add(StreamingLevel);
        ++SpawnedCount;

        // Also handles an instance that became visible before binding.
        HandleChunkLevelShown();
    }

    if (SpawnedCount > 0)
    {
        UpdateNavigationBounds();
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[WFC] Spawn complete. %d/%d tile levels spawned."
        ),
        SpawnedCount,
        GeneratedChunks.Num()
    );
}

void ALevelGenerator::ClearGeneratedChunks()
{
    OnGeneratedChunksClearing.Broadcast();

    for (FGeneratedChunkInstance& Instance :
        GeneratedChunkInstances)
    {
        if (IsValid(Instance.StreamingLevel))
        {
            Instance.StreamingLevel->OnLevelShown.RemoveDynamic(
                this,
                &ALevelGenerator::HandleChunkLevelShown
            );
        }
    }

    GeneratedChunkInstances.Reset();

    for (ULevelStreamingDynamic* StreamingLevel : SpawnedChunkLevels)
    {
        if (!IsValid(StreamingLevel))
        {
            continue;
        }

        StreamingLevel->SetShouldBeVisible(false);
        StreamingLevel->SetShouldBeLoaded(false);
    }

    const int32 RemovedLevelCount =
        SpawnedChunkLevels.Num();

    SpawnedChunkLevels.Reset();
    GeneratedChunks.Reset();

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[WFC] Cleared %d spawned tile levels "
            "and reset generated layout."
        ),
        RemovedLevelCount
    );
}

FVector ALevelGenerator::GridToWorldLocation(
    const FIntPoint& GridCoordinate
) const
{
    return FVector(
        static_cast<double>(GridCoordinate.X) * ChunkSize,
        static_cast<double>(GridCoordinate.Y) * ChunkSize,
        0.0
    );
}

FRotator ALevelGenerator::ChunkRotationToWorldRotation(
    EChunkRotation Rotation
)
{
    switch (Rotation)
    {
    case EChunkRotation::Degrees0:
        return FRotator(0.0, 0.0, 0.0);

    case EChunkRotation::Degrees90:
        return FRotator(0.0, 90.0, 0.0);

    case EChunkRotation::Degrees180:
        return FRotator(0.0, 180.0, 0.0);

    case EChunkRotation::Degrees270:
        return FRotator(0.0, -90.0, 0.0);

    default:
        return FRotator::ZeroRotator;
    }
}

bool ALevelGenerator::ValidateGeneratedChunks() const
{
    if (GeneratedChunks.Num() != GridWidth * GridHeight)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC] Expected %d generated chunks, got %d."
            ),
            GridWidth * GridHeight,
            GeneratedChunks.Num()
        );

        return false;
    }

    TMap<FIntPoint, const FGeneratedChunk*> ChunkByCoordinate;
    ChunkByCoordinate.Reserve(GeneratedChunks.Num());

    for (const FGeneratedChunk& Chunk : GeneratedChunks)
    {
        if (!IsValid(Chunk.Definition))
        {
            return false;
        }

        ChunkByCoordinate.Add(
            Chunk.GridCoordinate,
            &Chunk
        );
    }

    struct FDirectionInfo
    {
        EChunkConnectionDirection Direction;
        FIntPoint Offset;
    };

    static const FDirectionInfo Directions[] =
    {
        {
            EChunkConnectionDirection::North,
            FIntPoint(0, 1)
        },
        {
            EChunkConnectionDirection::East,
            FIntPoint(1, 0)
        }
    };

    /*
     * North and East are enough here.
     * Every internal edge is otherwise checked twice.
     */
    for (const FGeneratedChunk& Chunk : GeneratedChunks)
    {
        for (const FDirectionInfo& Info : Directions)
        {
            const FIntPoint NeighborCoordinate =
                Chunk.GridCoordinate + Info.Offset;

            const FGeneratedChunk* const* NeighborPtr =
                ChunkByCoordinate.Find(NeighborCoordinate);

            if (!NeighborPtr)
            {
                continue;
            }

            const FGeneratedChunk* Neighbor =
                *NeighborPtr;

            if (!Neighbor || !IsValid(Neighbor->Definition))
            {
                return false;
            }

            const EChunkEdgeType Edge =
                Chunk.Definition->GetEdge(
                    Info.Direction,
                    Chunk.Rotation
                );

            const EChunkConnectionDirection OppositeDirection =
                ULevelChunkDefinition::GetOppositeDirection(
                    Info.Direction
                );

            const EChunkEdgeType NeighborEdge =
                Neighbor->Definition->GetEdge(
                    OppositeDirection,
                    Neighbor->Rotation
                );

            if (Edge != NeighborEdge)
            {
                UE_LOG(
                    LogTemp,
                    Error,
                    TEXT(
                        "[WFC] Final layout mismatch between "
                        "Grid(%d,%d) and Grid(%d,%d)."
                    ),
                    Chunk.GridCoordinate.X,
                    Chunk.GridCoordinate.Y,
                    Neighbor->GridCoordinate.X,
                    Neighbor->GridCoordinate.Y
                );

                return false;
            }
        }
    }

    return true;
}

void ALevelGenerator::HandleChunkLevelShown()
{
    for (FGeneratedChunkInstance& Instance :
        GeneratedChunkInstances)
    {
        if (Instance.bReadyBroadcast ||
            !IsValid(Instance.StreamingLevel) ||
            !IsValid(Instance.Definition) ||
            !Instance.StreamingLevel->IsLevelVisible() ||
            !Instance.StreamingLevel->GetLoadedLevel())
        {
            continue;
        }

        // Set this before broadcasting so listeners cannot cause
        // this instance to be announced twice.
        Instance.bReadyBroadcast = true;

        OnGeneratedChunkReady.Broadcast(
            Instance.StreamingLevel,
            Instance.Definition,
            Instance.GridCoordinate
        );
    }
}

void ALevelGenerator::UpdateNavigationBounds()
{
    if (!GetWorld() ||
        !GetWorld()->IsGameWorld() ||
        !HasAuthority() ||
        !IsValid(NavigationBoundsVolume))
    {
        return;
    }

    UBrushComponent* Brush =
        NavigationBoundsVolume->GetBrushComponent();

    if (!Brush)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[WFC NAV] Navigation volume has no brush.")
        );
        return;
    }

    /*
     * Read the brush's size in its local, unscaled space.
     * The volume should have zero rotation.
     */
    const FVector LocalBrushSize =
        Brush->CalcBounds(FTransform::Identity).BoxExtent * 2.0;

    if (LocalBrushSize.X <= 0.0 ||
        LocalBrushSize.Y <= 0.0 ||
        LocalBrushSize.Z <= 0.0)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[WFC NAV] Navigation volume brush has invalid size.")
        );
        return;
    }

    /*
     * GridToWorldLocation() currently places tile origins at:
     * (0,0), (ChunkSize,0), (0,ChunkSize), ...
     *
     * This assumes a tile extends roughly half a ChunkSize
     * in each direction around its origin.
     */
    const float NavWidth =
        GridWidth * ChunkSize + 2.0f * NavigationMargin;

    const float NavDepth =
        GridHeight * ChunkSize + 2.0f * NavigationMargin;

    const FVector NewCenter(
        (GridWidth - 1) * ChunkSize * 0.5f,
        (GridHeight - 1) * ChunkSize * 0.5f,
        NavigationCenterZ
    );

    const FVector NewScale(
        NavWidth / LocalBrushSize.X,
        NavDepth / LocalBrushSize.Y,
        NavigationHeight / LocalBrushSize.Z
    );

    NavigationBoundsVolume->SetActorLocation(NewCenter);
    NavigationBoundsVolume->SetActorScale3D(NewScale);

    UNavigationSystemV1* NavSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(
            GetWorld()
        );

    if (NavSystem)
    {
        NavSystem->OnNavigationBoundsUpdated(
            NavigationBoundsVolume
        );
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[WFC NAV] Bounds center=%s size=(%.0f, %.0f, %.0f)"
        ),
        *NewCenter.ToString(),
        NavWidth,
        NavDepth,
        NavigationHeight
    );
}

