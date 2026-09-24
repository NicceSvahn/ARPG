#include "WFCLevelSolver.h"

#include "../LevelChunkDefinition.h"

namespace
{
    constexpr EChunkConnectionDirection Directions[] =
    {
        EChunkConnectionDirection::North,
        EChunkConnectionDirection::East,
        EChunkConnectionDirection::South,
        EChunkConnectionDirection::West
    };
}

bool FWFCLevelSolver::Solve(
    int32 Width,
    int32 Height,
    const TArray<ULevelChunkDefinition*>& Definitions,
    int32 Seed
)
{
    RandomStream.Initialize(Seed);

    if (!Initialize(Width, Height, Definitions))
    {
        return false;
    }

    return RunCollapse();
}

bool FWFCLevelSolver::Initialize(
    int32 Width,
    int32 Height,
    const TArray<ULevelChunkDefinition*>& Definitions
)
{
    if (Width <= 0 || Height <= 0)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[WFC] Grid dimensions must be greater than zero.")
        );

        return false;
    }

    if (Definitions.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[WFC] No tile definitions provided.")
        );

        return false;
    }

    GridWidth = Width;
    GridHeight = Height;

    Cells.Reset();
    Cells.Reserve(GridWidth * GridHeight);

    TArray<FWFCState> AllStates;

    for (ULevelChunkDefinition* Definition : Definitions)
    {
        if (!IsValid(Definition))
        {
            continue;
        }

        for (const EChunkRotation Rotation : Definition->AllowedRotations)
        {
            FWFCState State;
            State.Definition = Definition;
            State.Rotation = Rotation;

            AllStates.Add(State);
        }
    }

    if (AllStates.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[WFC] Tile definitions produced no valid states.")
        );

        return false;
    }

    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            FWFCCell Cell;
            Cell.Coordinate = FIntPoint(X, Y);
            Cell.PossibleStates = AllStates;

            Cells.Add(MoveTemp(Cell));
        }
    }

    if (!ApplyBoundaryConstraints())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[WFC] Failed to apply boundary constraints.")
        );

        return false;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[WFC] Boundary constraints applied.")
    );

    return true;
}

bool FWFCLevelSolver::RunCollapse()
{
    if (!CanStillBecomeFullyConnected())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC] Initial state cannot form a fully connected layout."
            )
        );

        return false;
    }

    if (!SolveRecursive())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC] No fully connected solution exists "
                "for the current grid and tile set."
            )
        );

        return false;
    }

    return true;
}

bool FWFCLevelSolver::SolveRecursive()
{
    if (!CanStillBecomeFullyConnected())
    {
        return false;
    }

    const int32 CellIndex = FindLowestEntropyCell();

    // Every cell has collapsed. Validate the finished layout.
    if (CellIndex == INDEX_NONE)
    {
        return ValidateSolution()
            && ValidateConnectivity();
    }

    if (!Cells.IsValidIndex(CellIndex))
    {
        return false;
    }

    const TArray<FWFCState> CandidateStates =
        Cells[CellIndex].PossibleStates;

    if (CandidateStates.IsEmpty())
    {
        return false;
    }

    /*
     * Randomize candidate order using the seeded random stream.
     *
     * This keeps generation deterministic for a given seed while
     * allowing backtracking to try alternate states.
     */
    TArray<FWFCState> ShuffledStates = CandidateStates;

    for (int32 Index = ShuffledStates.Num() - 1; Index > 0; --Index)
    {
        const int32 SwapIndex =
            RandomStream.RandRange(0, Index);

        ShuffledStates.Swap(Index, SwapIndex);
    }

    for (const FWFCState& CandidateState : ShuffledStates)
    {
        const TArray<FWFCCell> SavedCells = Cells;

        Cells[CellIndex].PossibleStates.Reset();
        Cells[CellIndex].PossibleStates.Add(CandidateState);

        if (PropagateFrom(CellIndex)
            && CanStillBecomeFullyConnected()
            && SolveRecursive())
        {
            return true;
        }

        // Restore the previous solver state and try another candidate.
        Cells = SavedCells;
    }

    return false;
}

int32 FWFCLevelSolver::FindLowestEntropyCell()
{
    int32 LowestEntropy = MAX_int32;

    TArray<int32> Candidates;

    for (int32 Index = 0; Index < Cells.Num(); ++Index)
    {
        const FWFCCell& Cell = Cells[Index];

        if (Cell.IsContradiction())
        {
            return Index;
        }

        const int32 Entropy = Cell.GetEntropy();

        if (Entropy <= 1)
        {
            continue;
        }

        if (Entropy < LowestEntropy)
        {
            LowestEntropy = Entropy;

            Candidates.Reset();
            Candidates.Add(Index);
        }
        else if (Entropy == LowestEntropy)
        {
            Candidates.Add(Index);
        }
    }

    if (Candidates.IsEmpty())
    {
        return INDEX_NONE;
    }

    return Candidates[
        RandomStream.RandRange(0, Candidates.Num() - 1)
    ];
}

bool FWFCLevelSolver::PropagateFrom(int32 CellIndex)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return false;
    }

    TArray<int32> Queue;
    Queue.Add(CellIndex);

    int32 QueuePosition = 0;

    while (QueuePosition < Queue.Num())
    {
        const int32 CurrentIndex =
            Queue[QueuePosition++];

        const FWFCCell& CurrentCell =
            Cells[CurrentIndex];

        for (const EChunkConnectionDirection Direction : Directions)
        {
            const FIntPoint NeighborCoordinate =
                CurrentCell.Coordinate
                + DirectionToOffset(Direction);

            if (!IsInsideGrid(NeighborCoordinate))
            {
                continue;
            }

            const int32 NeighborIndex =
                CoordinateToIndex(NeighborCoordinate);

            FWFCCell& Neighbor =
                Cells[NeighborIndex];

            bool bChanged = false;

            for (
                int32 StateIndex = Neighbor.PossibleStates.Num() - 1;
                StateIndex >= 0;
                --StateIndex
                )
            {
                const FWFCState& NeighborState =
                    Neighbor.PossibleStates[StateIndex];

                bool bHasSupport = false;

                for (const FWFCState& CurrentState :
                    CurrentCell.PossibleStates)
                {
                    if (AreStatesCompatible(
                        CurrentState,
                        NeighborState,
                        Direction
                    ))
                    {
                        bHasSupport = true;
                        break;
                    }
                }

                if (!bHasSupport)
                {
                    Neighbor.PossibleStates.RemoveAt(StateIndex);
                    bChanged = true;
                }
            }

            if (Neighbor.IsContradiction())
            {
                return false;
            }

            if (bChanged)
            {
                Queue.AddUnique(NeighborIndex);
            }
        }
    }

    return true;
}

bool FWFCLevelSolver::AreStatesCompatible(
    const FWFCState& A,
    const FWFCState& B,
    EChunkConnectionDirection DirectionFromA
) const
{
    if (!IsValid(A.Definition)
        || !IsValid(B.Definition))
    {
        return false;
    }

    const EChunkConnectionDirection DirectionFromB =
        ULevelChunkDefinition::GetOppositeDirection(
            DirectionFromA
        );

    const EChunkEdgeType EdgeA =
        A.Definition->GetEdge(
            DirectionFromA,
            A.Rotation
        );

    const EChunkEdgeType EdgeB =
        B.Definition->GetEdge(
            DirectionFromB,
            B.Rotation
        );

    return EdgeA == EdgeB;
}

bool FWFCLevelSolver::ApplyBoundaryConstraints()
{
    for (FWFCCell& Cell : Cells)
    {
        const bool bIsWestBoundary =
            Cell.Coordinate.X == 0;

        const bool bIsEastBoundary =
            Cell.Coordinate.X == GridWidth - 1;

        const bool bIsSouthBoundary =
            Cell.Coordinate.Y == 0;

        const bool bIsNorthBoundary =
            Cell.Coordinate.Y == GridHeight - 1;

        for (
            int32 StateIndex = Cell.PossibleStates.Num() - 1;
            StateIndex >= 0;
            --StateIndex
            )
        {
            const FWFCState& State =
                Cell.PossibleStates[StateIndex];

            if (!IsValid(State.Definition))
            {
                Cell.PossibleStates.RemoveAt(StateIndex);
                continue;
            }

            const bool bInvalid =
                (bIsNorthBoundary
                    && State.Definition->GetEdge(
                        EChunkConnectionDirection::North,
                        State.Rotation
                    ) == EChunkEdgeType::Open)
                ||
                (bIsEastBoundary
                    && State.Definition->GetEdge(
                        EChunkConnectionDirection::East,
                        State.Rotation
                    ) == EChunkEdgeType::Open)
                ||
                (bIsSouthBoundary
                    && State.Definition->GetEdge(
                        EChunkConnectionDirection::South,
                        State.Rotation
                    ) == EChunkEdgeType::Open)
                ||
                (bIsWestBoundary
                    && State.Definition->GetEdge(
                        EChunkConnectionDirection::West,
                        State.Rotation
                    ) == EChunkEdgeType::Open);

            if (bInvalid)
            {
                Cell.PossibleStates.RemoveAt(StateIndex);
            }
        }

        if (Cell.IsContradiction())
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[WFC] Boundary contradiction at Cell(%d,%d)."
                ),
                Cell.Coordinate.X,
                Cell.Coordinate.Y
            );

            return false;
        }
    }

    return true;
}

bool FWFCLevelSolver::CanPotentiallyConnect(
    const FWFCCell& A,
    const FWFCCell& B,
    EChunkConnectionDirection DirectionFromA
) const
{
    const EChunkConnectionDirection DirectionFromB =
        ULevelChunkDefinition::GetOppositeDirection(
            DirectionFromA
        );

    for (const FWFCState& StateA : A.PossibleStates)
    {
        if (!IsValid(StateA.Definition))
        {
            continue;
        }

        if (StateA.Definition->GetEdge(
            DirectionFromA,
            StateA.Rotation
        ) != EChunkEdgeType::Open)
        {
            continue;
        }

        for (const FWFCState& StateB : B.PossibleStates)
        {
            if (!IsValid(StateB.Definition))
            {
                continue;
            }

            if (StateB.Definition->GetEdge(
                DirectionFromB,
                StateB.Rotation
            ) == EChunkEdgeType::Open)
            {
                return true;
            }
        }
    }

    return false;
}

bool FWFCLevelSolver::CanStillBecomeFullyConnected() const
{
    if (Cells.IsEmpty())
    {
        return false;
    }

    const int32 StartIndex =
        CoordinateToIndex(FIntPoint::ZeroValue);

    if (!Cells.IsValidIndex(StartIndex))
    {
        return false;
    }

    TArray<int32> Queue;
    TSet<int32> Visited;

    Queue.Add(StartIndex);
    Visited.Add(StartIndex);

    int32 QueuePosition = 0;

    while (QueuePosition < Queue.Num())
    {
        const int32 CurrentIndex =
            Queue[QueuePosition++];

        const FWFCCell& CurrentCell =
            Cells[CurrentIndex];

        for (const EChunkConnectionDirection Direction : Directions)
        {
            const FIntPoint NeighborCoordinate =
                CurrentCell.Coordinate
                + DirectionToOffset(Direction);

            if (!IsInsideGrid(NeighborCoordinate))
            {
                continue;
            }

            const int32 NeighborIndex =
                CoordinateToIndex(NeighborCoordinate);

            if (Visited.Contains(NeighborIndex))
            {
                continue;
            }

            const FWFCCell& Neighbor =
                Cells[NeighborIndex];

            if (!CanPotentiallyConnect(
                CurrentCell,
                Neighbor,
                Direction
            ))
            {
                continue;
            }

            Visited.Add(NeighborIndex);
            Queue.Add(NeighborIndex);
        }
    }

    return Visited.Num() == Cells.Num();
}

bool FWFCLevelSolver::ValidateSolution() const
{
    for (const FWFCCell& Cell : Cells)
    {
        if (!Cell.IsCollapsed())
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[WFC VALIDATION] Cell(%d,%d) is not collapsed. "
                    "Entropy=%d"
                ),
                Cell.Coordinate.X,
                Cell.Coordinate.Y,
                Cell.GetEntropy()
            );

            return false;
        }

        const FWFCState& State =
            Cell.PossibleStates[0];

        if (!IsValid(State.Definition))
        {
            return false;
        }

        for (const EChunkConnectionDirection Direction : Directions)
        {
            const FIntPoint NeighborCoordinate =
                Cell.Coordinate
                + DirectionToOffset(Direction);

            if (!IsInsideGrid(NeighborCoordinate))
            {
                if (State.Definition->GetEdge(
                    Direction,
                    State.Rotation
                ) != EChunkEdgeType::Closed)
                {
                    UE_LOG(
                        LogTemp,
                        Error,
                        TEXT(
                            "[WFC VALIDATION] Cell(%d,%d) [%s Rot=%d] "
                            "%s points outside the grid."
                        ),
                        Cell.Coordinate.X,
                        Cell.Coordinate.Y,
                        *State.Definition->ChunkId.ToString(),
                        static_cast<int32>(State.Rotation) * 90,
                        DirectionToString(Direction)
                    );

                    return false;
                }

                continue;
            }

            /*
             * Only validate North and East.
             * South and West are the same connections in reverse.
             */
            if (Direction != EChunkConnectionDirection::North
                && Direction != EChunkConnectionDirection::East)
            {
                continue;
            }

            const int32 NeighborIndex =
                CoordinateToIndex(NeighborCoordinate);

            if (!Cells.IsValidIndex(NeighborIndex))
            {
                return false;
            }

            const FWFCCell& Neighbor =
                Cells[NeighborIndex];

            if (!Neighbor.IsCollapsed())
            {
                return false;
            }

            const FWFCState& NeighborState =
                Neighbor.PossibleStates[0];

            if (!IsValid(NeighborState.Definition))
            {
                return false;
            }

            if (!AreStatesCompatible(
                State,
                NeighborState,
                Direction
            ))
            {
                UE_LOG(
                    LogTemp,
                    Error,
                    TEXT(
                        "[WFC VALIDATION] Edge mismatch: "
                        "Cell(%d,%d) [%s Rot=%d] %s=%s <-> "
                        "Cell(%d,%d) [%s Rot=%d] %s=%s"
                    ),
                    Cell.Coordinate.X,
                    Cell.Coordinate.Y,
                    *State.Definition->ChunkId.ToString(),
                    static_cast<int32>(State.Rotation) * 90,
                    DirectionToString(Direction),
                    EdgeToString(
                        State.Definition->GetEdge(
                            Direction,
                            State.Rotation
                        )
                    ),
                    Neighbor.Coordinate.X,
                    Neighbor.Coordinate.Y,
                    *NeighborState.Definition->ChunkId.ToString(),
                    static_cast<int32>(NeighborState.Rotation) * 90,
                    DirectionToString(
                        ULevelChunkDefinition::GetOppositeDirection(
                            Direction
                        )
                    ),
                    EdgeToString(
                        NeighborState.Definition->GetEdge(
                            ULevelChunkDefinition::GetOppositeDirection(
                                Direction
                            ),
                            NeighborState.Rotation
                        )
                    )
                );

                return false;
            }
        }
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[WFC VALIDATION] All tile constraints are valid.")
    );

    return true;
}

bool FWFCLevelSolver::ValidateConnectivity() const
{
    if (Cells.IsEmpty())
    {
        return false;
    }

    const int32 StartIndex =
        CoordinateToIndex(FIntPoint::ZeroValue);

    if (!Cells.IsValidIndex(StartIndex))
    {
        return false;
    }

    TArray<int32> Queue;
    TSet<int32> Visited;

    Queue.Add(StartIndex);
    Visited.Add(StartIndex);

    int32 QueuePosition = 0;

    while (QueuePosition < Queue.Num())
    {
        const int32 CurrentIndex =
            Queue[QueuePosition++];

        const FWFCCell& CurrentCell =
            Cells[CurrentIndex];

        if (!CurrentCell.IsCollapsed())
        {
            return false;
        }

        const FWFCState& CurrentState =
            CurrentCell.PossibleStates[0];

        if (!IsValid(CurrentState.Definition))
        {
            return false;
        }

        for (const EChunkConnectionDirection Direction : Directions)
        {
            if (CurrentState.Definition->GetEdge(
                Direction,
                CurrentState.Rotation
            ) != EChunkEdgeType::Open)
            {
                continue;
            }

            const FIntPoint NeighborCoordinate =
                CurrentCell.Coordinate
                + DirectionToOffset(Direction);

            if (!IsInsideGrid(NeighborCoordinate))
            {
                continue;
            }

            const int32 NeighborIndex =
                CoordinateToIndex(NeighborCoordinate);

            if (Visited.Contains(NeighborIndex))
            {
                continue;
            }

            const FWFCCell& Neighbor =
                Cells[NeighborIndex];

            if (!Neighbor.IsCollapsed())
            {
                return false;
            }

            const FWFCState& NeighborState =
                Neighbor.PossibleStates[0];

            if (!IsValid(NeighborState.Definition))
            {
                return false;
            }

            const EChunkConnectionDirection OppositeDirection =
                ULevelChunkDefinition::GetOppositeDirection(
                    Direction
                );

            if (NeighborState.Definition->GetEdge(
                OppositeDirection,
                NeighborState.Rotation
            ) != EChunkEdgeType::Open)
            {
                continue;
            }

            Visited.Add(NeighborIndex);
            Queue.Add(NeighborIndex);
        }
    }

    if (Visited.Num() != Cells.Num())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC CONNECTIVITY] Layout is disconnected. "
                "Reachable=%d/%d"
            ),
            Visited.Num(),
            Cells.Num()
        );

        return false;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[WFC CONNECTIVITY] Layout is fully connected. "
            "%d/%d cells reachable."
        ),
        Visited.Num(),
        Cells.Num()
    );

    return true;
}

bool FWFCLevelSolver::IsInsideGrid(
    const FIntPoint& Coordinate
) const
{
    return Coordinate.X >= 0
        && Coordinate.X < GridWidth
        && Coordinate.Y >= 0
        && Coordinate.Y < GridHeight;
}

int32 FWFCLevelSolver::CoordinateToIndex(
    const FIntPoint& Coordinate
) const
{
    return Coordinate.Y * GridWidth
        + Coordinate.X;
}

FIntPoint FWFCLevelSolver::DirectionToOffset(
    EChunkConnectionDirection Direction
)
{
    switch (Direction)
    {
    case EChunkConnectionDirection::North:
        return FIntPoint(0, 1);

    case EChunkConnectionDirection::East:
        return FIntPoint(1, 0);

    case EChunkConnectionDirection::South:
        return FIntPoint(0, -1);

    case EChunkConnectionDirection::West:
        return FIntPoint(-1, 0);

    default:
        return FIntPoint::ZeroValue;
    }
}

const TCHAR* FWFCLevelSolver::EdgeToString(
    EChunkEdgeType Edge
)
{
    switch (Edge)
    {
    case EChunkEdgeType::Open:
        return TEXT("Open");

    case EChunkEdgeType::Closed:
        return TEXT("Closed");

    default:
        return TEXT("Unknown");
    }
}

const TCHAR* FWFCLevelSolver::DirectionToString(
    EChunkConnectionDirection Direction
)
{
    switch (Direction)
    {
    case EChunkConnectionDirection::North:
        return TEXT("North");

    case EChunkConnectionDirection::East:
        return TEXT("East");

    case EChunkConnectionDirection::South:
        return TEXT("South");

    case EChunkConnectionDirection::West:
        return TEXT("West");

    default:
        return TEXT("Unknown");
    }
}