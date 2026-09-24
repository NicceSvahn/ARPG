#include "LevelGenerator.h"

#include "Engine/LevelStreamingDynamic.h"
#include "WFC/WFCLevelSolver.h"

ALevelGenerator::ALevelGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ALevelGenerator::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[WFC RUNTIME] LevelGenerator BeginPlay")
    );

    if (bGenerateOnBeginPlay)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[WFC RUNTIME] Generating runtime level...")
        );

        GenerateLevel();
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
                "[WFC] Cannot generate while chunk levels "
                "are spawned. Run Clear Generated Chunks first."
            )
        );

        return;
    }

    GeneratedChunks.Empty();

    /*
     * Convert our reflected TObjectPtr array into the raw
     * pointer array expected by the solver.
     */
    TArray<ULevelChunkDefinition*> Definitions;

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
            TEXT("[WFC] No WFC tile definitions configured.")
        );

        return;
    }

    FWFCLevelSolver Solver;

    const bool bSuccess =
        Solver.Solve(
            GridWidth,
            GridHeight,
            Definitions,
            GenerationSeed
        );

    if (!bSuccess)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC] Generation failed due to a contradiction. "
                "Seed=%d"
            ),
            GenerationSeed
        );

        return;
    }

    /*
     * Convert the solved WFC grid into the format our
     * existing level streaming code understands.
     */
    for (const FWFCCell& Cell : Solver.GetCells())
    {
        if (!Cell.IsCollapsed())
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "[WFC] Cell (%d,%d) was not collapsed."
                ),
                Cell.Coordinate.X,
                Cell.Coordinate.Y
            );

            continue;
        }

        const FWFCState& State =
            Cell.PossibleStates[0];

        if (!IsValid(State.Definition))
        {
            continue;
        }

        FGeneratedChunk GeneratedChunk;

        GeneratedChunk.Definition =
            State.Definition;

        GeneratedChunk.GridCoordinate =
            Cell.Coordinate;

        GeneratedChunk.Rotation =
            State.Rotation;

        GeneratedChunks.Add(
            GeneratedChunk
        );

        UE_LOG(
            LogTemp,
            Verbose,
            TEXT(
                "[WFC] Cell (%d,%d) = %s, Rotation=%d"
            ),
            Cell.Coordinate.X,
            Cell.Coordinate.Y,
            *State.Definition->ChunkId.ToString(),
            static_cast<int32>(State.Rotation) * 90
        );
    }

    /*
     * Validate the final layout that LevelGenerator
     * is actually going to spawn.
     */
    if (!ValidateGeneratedChunks())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC] GeneratedChunks failed final validation."
            )
        );

        GeneratedChunks.Empty();
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
                "Run Generate Level first."
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

        const EChunkEdgeType NorthEdge =
            Chunk.Definition->GetEdge(
                EChunkConnectionDirection::North,
                Chunk.Rotation
            );

        const EChunkEdgeType EastEdge =
            Chunk.Definition->GetEdge(
                EChunkConnectionDirection::East,
                Chunk.Rotation
            );

        const EChunkEdgeType SouthEdge =
            Chunk.Definition->GetEdge(
                EChunkConnectionDirection::South,
                Chunk.Rotation
            );

        const EChunkEdgeType WestEdge =
            Chunk.Definition->GetEdge(
                EChunkConnectionDirection::West,
                Chunk.Rotation
            );

        auto EdgeToString =
            [](EChunkEdgeType Edge) -> const TCHAR*
            {
                return Edge == EChunkEdgeType::Open
                    ? TEXT("OPEN")
                    : TEXT("CLOSED");
            };

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[WFC PHYSICAL CHECK] "
                "%s Grid(%d,%d) Rot=%d Yaw=%.0f | "
                "N=%s E=%s S=%s W=%s"
            ),
            *Chunk.Definition->ChunkId.ToString(),
            Chunk.GridCoordinate.X,
            Chunk.GridCoordinate.Y,
            static_cast<int32>(Chunk.Rotation),
            WorldRotation.Yaw,
            EdgeToString(NorthEdge),
            EdgeToString(EastEdge),
            EdgeToString(SouthEdge),
            EdgeToString(WestEdge)
        );

        /*
         * Debug:
         *
         * This lets us compare the logical WFC rotation
         * with the actual Unreal world rotation.
         */
        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[WFC ROTATION] Tile=%s "
                "Grid=(%d,%d) "
                "LogicalRotation=%d "
                "WorldYaw=%.0f"
            ),
            *Chunk.Definition->ChunkId.ToString(),
            Chunk.GridCoordinate.X,
            Chunk.GridCoordinate.Y,
            static_cast<int32>(Chunk.Rotation),
            WorldRotation.Yaw
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

        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[STREAM TRANSFORM] %s Grid(%d,%d) "
                "RequestedYaw=%.0f "
                "ActualLevelYaw=%.0f"
            ),
            *Chunk.Definition->ChunkId.ToString(),
            Chunk.GridCoordinate.X,
            Chunk.GridCoordinate.Y,
            WorldRotation.Yaw,
            StreamingLevel->LevelTransform.Rotator().Yaw
        );

        if (!bLoadSuccess ||
            !IsValid(StreamingLevel))
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

        SpawnedChunkLevels.Add(
            StreamingLevel
        );

        ++SpawnedCount;

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[WFC] Spawned %s at Grid(%d,%d) "
                "Rotation=%d -> World(%.0f,%.0f,%.0f), Yaw=%.0f"
            ),
            *Chunk.Definition->ChunkId.ToString(),
            Chunk.GridCoordinate.X,
            Chunk.GridCoordinate.Y,
            static_cast<int32>(Chunk.Rotation),
            WorldLocation.X,
            WorldLocation.Y,
            WorldLocation.Z,
            WorldRotation.Yaw
        );
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[WFC] Spawn complete. "
            "%d/%d tile levels spawned."
        ),
        SpawnedCount,
        GeneratedChunks.Num()
    );
}

void ALevelGenerator::ClearGeneratedChunks()
{
    for (ULevelStreamingDynamic* StreamingLevel :
        SpawnedChunkLevels)
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

    SpawnedChunkLevels.Empty();
    GeneratedChunks.Empty();

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
        static_cast<double>(GridCoordinate.X) *
        ChunkSize,

        static_cast<double>(GridCoordinate.Y) *
        ChunkSize,

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
        return FRotator(
            0.0,
            0.0,
            0.0
        );

    case EChunkRotation::Degrees90:
        return FRotator(
            0.0,
            90.0,
            0.0
        );

    case EChunkRotation::Degrees180:
        return FRotator(
            0.0,
            180.0,
            0.0
        );

    case EChunkRotation::Degrees270:
        return FRotator(
            0.0,
            -90.0,
            0.0
        );

    default:
        return FRotator::ZeroRotator;
    }
}

bool ALevelGenerator::ValidateGeneratedChunks() const
{
    auto FindChunk =
        [this](const FIntPoint& Coordinate)
        -> const FGeneratedChunk*
        {
            for (const FGeneratedChunk& Chunk : GeneratedChunks)
            {
                if (Chunk.GridCoordinate == Coordinate)
                {
                    return &Chunk;
                }
            }

            return nullptr;
        };

    struct FDirectionInfo
    {
        EChunkConnectionDirection Direction;
        FIntPoint Offset;
    };

    static const FDirectionInfo Directions[] =
    {
        { EChunkConnectionDirection::North, FIntPoint(0, 1) },
        { EChunkConnectionDirection::East,  FIntPoint(1, 0) },
        { EChunkConnectionDirection::South, FIntPoint(0, -1) },
        { EChunkConnectionDirection::West,  FIntPoint(-1, 0) }
    };

    bool bValid = true;

    for (const FGeneratedChunk& Chunk : GeneratedChunks)
    {
        if (!IsValid(Chunk.Definition))
        {
            bValid = false;
            continue;
        }

        for (const FDirectionInfo& Info : Directions)
        {
            const EChunkEdgeType Edge =
                Chunk.Definition->GetEdge(
                    Info.Direction,
                    Chunk.Rotation
                );

            const FIntPoint NeighborCoordinate =
                Chunk.GridCoordinate + Info.Offset;

            const FGeneratedChunk* Neighbor =
                FindChunk(NeighborCoordinate);

            if (!Neighbor)
            {
                if (Edge == EChunkEdgeType::Open)
                {
                    UE_LOG(
                        LogTemp,
                        Error,
                        TEXT(
                            "[FINAL VALIDATION] OUTSIDE OPEN: "
                            "Grid(%d,%d) %s Rot=%d Direction=%d"
                        ),
                        Chunk.GridCoordinate.X,
                        Chunk.GridCoordinate.Y,
                        *Chunk.Definition->ChunkId.ToString(),
                        static_cast<int32>(Chunk.Rotation),
                        static_cast<int32>(Info.Direction)
                    );

                    bValid = false;
                }

                continue;
            }

            if (!IsValid(Neighbor->Definition))
            {
                bValid = false;
                continue;
            }

            const EChunkConnectionDirection Opposite =
                ULevelChunkDefinition::GetOppositeDirection(
                    Info.Direction
                );

            const EChunkEdgeType NeighborEdge =
                Neighbor->Definition->GetEdge(
                    Opposite,
                    Neighbor->Rotation
                );

            if (Edge != NeighborEdge)
            {
                UE_LOG(
                    LogTemp,
                    Error,
                    TEXT(
                        "[FINAL VALIDATION] MISMATCH: "
                        "Grid(%d,%d) %s Rot=%d Edge=%d "
                        "<-> "
                        "Grid(%d,%d) %s Rot=%d Edge=%d"
                    ),
                    Chunk.GridCoordinate.X,
                    Chunk.GridCoordinate.Y,
                    *Chunk.Definition->ChunkId.ToString(),
                    static_cast<int32>(Chunk.Rotation),
                    static_cast<int32>(Edge),

                    Neighbor->GridCoordinate.X,
                    Neighbor->GridCoordinate.Y,
                    *Neighbor->Definition->ChunkId.ToString(),
                    static_cast<int32>(Neighbor->Rotation),
                    static_cast<int32>(NeighborEdge)
                );

                bValid = false;
            }
        }
    }

    if (bValid)
    {
        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[FINAL VALIDATION] GeneratedChunks layout is VALID."
            )
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[FINAL VALIDATION] GeneratedChunks layout is INVALID."
            )
        );
    }

    return bValid;
}