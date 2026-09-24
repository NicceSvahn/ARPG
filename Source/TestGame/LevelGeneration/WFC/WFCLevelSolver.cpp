#include "WFCLevelSolver.h"

#include "../LevelChunkDefinition.h"

bool FWFCLevelSolver::Solve(
    int32 Width,
    int32 Height,
    const TArray<ULevelChunkDefinition*>& Definitions,
    int32 Seed
)
{
    RandomStream.Initialize(Seed);

    if (!Initialize(
        Width,
        Height,
        Definitions
    ))
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
        return false;
    }

    if (Definitions.IsEmpty())
    {
        return false;
    }

    GridWidth = Width;
    GridHeight = Height;

    Cells.Empty();
    Cells.Reserve(GridWidth * GridHeight);

    TArray<FWFCState> AllStates;

    for (ULevelChunkDefinition* Definition : Definitions)
    {
        if (!Definition)
        {
            continue;
        }

        for (const EChunkRotation Rotation :
        Definition->AllowedRotations)
        {
            FWFCState State;
            State.Definition = Definition;
            State.Rotation = Rotation;

            AllStates.Add(State);
        }
    }

    if (AllStates.IsEmpty())
    {
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
                "[WFC CONNECTIVITY] Initial state cannot "
                "form a fully connected layout."
            )
        );

        return false;
    }

    const bool bSolved =
        SolveRecursive();

    if (!bSolved)
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

int32 FWFCLevelSolver::FindLowestEntropyCell()
{
    int32 LowestEntropy = MAX_int32;

    TArray<int32> Candidates;

    for (int32 Index = 0;
        Index < Cells.Num();
        ++Index)
    {
        const FWFCCell& Cell = Cells[Index];

        if (Cell.IsContradiction())
        {
            return Index;
        }

        const int32 Entropy =
            Cell.GetEntropy();

        if (Entropy <= 1)
        {
            continue;
        }

        if (Entropy < LowestEntropy)
        {
            LowestEntropy = Entropy;

            Candidates.Empty();
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
        RandomStream.RandRange(
            0,
            Candidates.Num() - 1
        )
    ];
}

bool FWFCLevelSolver::CollapseCell(
    int32 CellIndex
)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return false;
    }

    FWFCCell& Cell =
        Cells[CellIndex];

    if (Cell.PossibleStates.IsEmpty())
    {
        return false;
    }

    if (Cell.PossibleStates.Num() == 1)
    {
        return true;
    }

    /*
     * Weighted selection.
     */

    float TotalWeight = 0.0f;

    for (const FWFCState& State :
        Cell.PossibleStates)
    {
        if (!State.Definition)
        {
            continue;
        }

        TotalWeight +=
            FMath::Max(
                State.Definition->GenerationWeight,
                0.01f
            );
    }

    if (TotalWeight <= 0.0f)
    {
        return false;
    }

    float Roll =
        RandomStream.FRandRange(
            0.0f,
            TotalWeight
        );

    int32 SelectedIndex =
        Cell.PossibleStates.Num() - 1;

    for (int32 Index = 0;
        Index < Cell.PossibleStates.Num();
        ++Index)
    {
        const FWFCState& State =
            Cell.PossibleStates[Index];

        if (!State.Definition)
        {
            continue;
        }

        Roll -=
            FMath::Max(
                State.Definition->GenerationWeight,
                0.01f
            );

        if (Roll <= 0.0f)
        {
            SelectedIndex = Index;
            break;
        }
    }

    const FWFCState SelectedState =
        Cell.PossibleStates[SelectedIndex];

    Cell.PossibleStates.Empty();
    Cell.PossibleStates.Add(SelectedState);

    return true;
}

bool FWFCLevelSolver::PropagateFrom(
    int32 CellIndex
)
{
    if (!Cells.IsValidIndex(CellIndex))
    {
        return false;
    }

    TArray<int32> Queue;
    Queue.Add(CellIndex);

    int32 QueuePosition = 0;

    static const EChunkConnectionDirection Directions[] =
    {
        EChunkConnectionDirection::North,
        EChunkConnectionDirection::East,
        EChunkConnectionDirection::South,
        EChunkConnectionDirection::West
    };

    while (QueuePosition < Queue.Num())
    {
        const int32 CurrentIndex =
            Queue[QueuePosition++];

        const FWFCCell& CurrentCell =
            Cells[CurrentIndex];

        for (const EChunkConnectionDirection Direction :
        Directions)
        {
            const FIntPoint NeighborCoordinate =
                CurrentCell.Coordinate +
                DirectionToOffset(Direction);

            /*
             * Outside the WFC grid is ignored in V1.
             *
             * We will add explicit boundary constraints later.
             */
            if (!IsInsideGrid(NeighborCoordinate))
            {
                continue;
            }

            const int32 NeighborIndex =
                CoordinateToIndex(
                    NeighborCoordinate
                );

            FWFCCell& Neighbor =
                Cells[NeighborIndex];

            bool bChanged = false;

            for (int32 StateIndex =
                Neighbor.PossibleStates.Num() - 1;
                StateIndex >= 0;
                --StateIndex)
            {
                const FWFCState& NeighborState =
                    Neighbor.PossibleStates[
                        StateIndex
                    ];

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
                    Neighbor.PossibleStates.RemoveAt(
                        StateIndex
                    );

                    bChanged = true;
                }
            }

            if (Neighbor.IsContradiction())
            {
                return false;
            }

            if (bChanged)
            {
                Queue.AddUnique(
                    NeighborIndex
                );
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
    if (!A.Definition ||
        !B.Definition)
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

bool FWFCLevelSolver::HasCompatibleNeighborState(
    const FWFCState& State,
    const FWFCCell& Neighbor,
    EChunkConnectionDirection DirectionFromState
) const
{
    for (const FWFCState& NeighborState :
        Neighbor.PossibleStates)
    {
        if (AreStatesCompatible(
            State,
            NeighborState,
            DirectionFromState
        ))
        {
            return true;
        }
    }

    return false;
}

bool FWFCLevelSolver::IsInsideGrid(
    const FIntPoint& Coordinate
) const
{
    return
        Coordinate.X >= 0 &&
        Coordinate.X < GridWidth &&
        Coordinate.Y >= 0 &&
        Coordinate.Y < GridHeight;
}

int32 FWFCLevelSolver::CoordinateToIndex(
    const FIntPoint& Coordinate
) const
{
    return
        Coordinate.Y * GridWidth +
        Coordinate.X;
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

bool FWFCLevelSolver::ValidateSolution() const
{
    bool bValid = true;

    static const EChunkConnectionDirection Directions[] =
    {
        EChunkConnectionDirection::North,
        EChunkConnectionDirection::East,
        EChunkConnectionDirection::South,
        EChunkConnectionDirection::West
    };

    for (const FWFCCell& Cell : Cells)
    {
        if (!Cell.IsCollapsed())
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[WFC VALIDATION] Cell (%d,%d) "
                    "is not collapsed. Entropy=%d"
                ),
                Cell.Coordinate.X,
                Cell.Coordinate.Y,
                Cell.GetEntropy()
            );

            bValid = false;
            continue;
        }

        const FWFCState& State =
            Cell.PossibleStates[0];

        if (!State.Definition)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[WFC VALIDATION] Cell (%d,%d) "
                    "has no definition."
                ),
                Cell.Coordinate.X,
                Cell.Coordinate.Y
            );

            bValid = false;
            continue;
        }

        for (const EChunkConnectionDirection Direction :
        Directions)
        {
            const FIntPoint NeighborCoordinate =
                Cell.Coordinate +
                DirectionToOffset(Direction);

            if (!IsInsideGrid(NeighborCoordinate))
            {
                const EChunkEdgeType BoundaryEdge =
                    State.Definition->GetEdge(
                        Direction,
                        State.Rotation
                    );

                if (BoundaryEdge != EChunkEdgeType::Closed)
                {
                    UE_LOG(
                        LogTemp,
                        Error,
                        TEXT(
                            "[WFC VALIDATION ERROR] "
                            "Cell (%d,%d) [%s Rot=%d] "
                            "%s points outside grid but is Open."
                        ),
                        Cell.Coordinate.X,
                        Cell.Coordinate.Y,
                        *State.Definition->ChunkId.ToString(),
                        static_cast<int32>(State.Rotation) * 90,
                        DirectionToString(Direction)
                    );

                    bValid = false;
                }

                continue;
            }

            /*
             * Only check East and North.
             *
             * Otherwise every connection would be
             * validated twice.
             */
            if (Direction != EChunkConnectionDirection::North &&
                Direction != EChunkConnectionDirection::East)
            {
                continue;
            }

            const int32 NeighborIndex =
                CoordinateToIndex(
                    NeighborCoordinate
                );

            if (!Cells.IsValidIndex(NeighborIndex))
            {
                bValid = false;
                continue;
            }

            const FWFCCell& Neighbor =
                Cells[NeighborIndex];

            if (!Neighbor.IsCollapsed())
            {
                UE_LOG(
                    LogTemp,
                    Error,
                    TEXT(
                        "[WFC VALIDATION] Neighbor (%d,%d) "
                        "is not collapsed."
                    ),
                    Neighbor.Coordinate.X,
                    Neighbor.Coordinate.Y
                );

                bValid = false;
                continue;
            }

            const FWFCState& NeighborState =
                Neighbor.PossibleStates[0];

            if (!NeighborState.Definition)
            {
                bValid = false;
                continue;
            }

            const EChunkConnectionDirection OppositeDirection =
                ULevelChunkDefinition::GetOppositeDirection(
                    Direction
                );

            const EChunkEdgeType EdgeA =
                State.Definition->GetEdge(
                    Direction,
                    State.Rotation
                );

            const EChunkEdgeType EdgeB =
                NeighborState.Definition->GetEdge(
                    OppositeDirection,
                    NeighborState.Rotation
                );

            if (EdgeA != EdgeB)
            {
                UE_LOG(
                    LogTemp,
                    Error,
                    TEXT(
                        "[WFC VALIDATION ERROR] "
                        "Cell (%d,%d) [%s Rot=%d] %s=%s "
                        "<-> "
                        "Cell (%d,%d) [%s Rot=%d] %s=%s"
                    ),
                    Cell.Coordinate.X,
                    Cell.Coordinate.Y,
                    *State.Definition->ChunkId.ToString(),
                    static_cast<int32>(State.Rotation) * 90,
                    DirectionToString(Direction),
                    EdgeToString(EdgeA),

                    Neighbor.Coordinate.X,
                    Neighbor.Coordinate.Y,
                    *NeighborState.Definition->ChunkId.ToString(),
                    static_cast<int32>(NeighborState.Rotation) * 90,
                    DirectionToString(OppositeDirection),
                    EdgeToString(EdgeB)
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
                "[WFC VALIDATION] Solution is valid. "
                "All internal edges match."
            )
        );
    }
    else
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[WFC VALIDATION] Solution is INVALID."
            )
        );
    }

    return bValid;
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

        for (int32 StateIndex =
            Cell.PossibleStates.Num() - 1;
            StateIndex >= 0;
            --StateIndex)
        {
            const FWFCState& State =
                Cell.PossibleStates[StateIndex];

            if (!State.Definition)
            {
                Cell.PossibleStates.RemoveAt(StateIndex);
                continue;
            }

            bool bInvalid = false;

            /*
             * Nothing may connect outside the WFC grid.
             */

            if (bIsNorthBoundary &&
                State.Definition->GetEdge(
                    EChunkConnectionDirection::North,
                    State.Rotation
                ) == EChunkEdgeType::Open)
            {
                bInvalid = true;
            }

            if (bIsEastBoundary &&
                State.Definition->GetEdge(
                    EChunkConnectionDirection::East,
                    State.Rotation
                ) == EChunkEdgeType::Open)
            {
                bInvalid = true;
            }

            if (bIsSouthBoundary &&
                State.Definition->GetEdge(
                    EChunkConnectionDirection::South,
                    State.Rotation
                ) == EChunkEdgeType::Open)
            {
                bInvalid = true;
            }

            if (bIsWestBoundary &&
                State.Definition->GetEdge(
                    EChunkConnectionDirection::West,
                    State.Rotation
                ) == EChunkEdgeType::Open)
            {
                bInvalid = true;
            }

            if (bInvalid)
            {
                Cell.PossibleStates.RemoveAt(StateIndex);
            }
        }

        /*
         * A boundary constraint must never leave a cell
         * without any possible tile.
         */
        if (Cell.PossibleStates.IsEmpty())
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

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[WFC] Boundary constraints complete. "
            "All outward-facing states removed."
        )
    );

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
        if (!StateA.Definition)
        {
            continue;
        }

        const EChunkEdgeType EdgeA =
            StateA.Definition->GetEdge(
                DirectionFromA,
                StateA.Rotation
            );

        if (EdgeA != EChunkEdgeType::Open)
        {
            continue;
        }

        for (const FWFCState& StateB : B.PossibleStates)
        {
            if (!StateB.Definition)
            {
                continue;
            }

            const EChunkEdgeType EdgeB =
                StateB.Definition->GetEdge(
                    DirectionFromB,
                    StateB.Rotation
                );

            if (EdgeB == EChunkEdgeType::Open)
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

    /*
     * Start from Grid(0,0).
     */
    const int32 StartIndex =
        CoordinateToIndex(FIntPoint(0, 0));

    if (!Cells.IsValidIndex(StartIndex))
    {
        return false;
    }

    TArray<int32> Queue;
    TSet<int32> Visited;

    Queue.Add(StartIndex);
    Visited.Add(StartIndex);

    int32 QueuePosition = 0;

    static const EChunkConnectionDirection Directions[] =
    {
        EChunkConnectionDirection::North,
        EChunkConnectionDirection::East,
        EChunkConnectionDirection::South,
        EChunkConnectionDirection::West
    };

    while (QueuePosition < Queue.Num())
    {
        const int32 CurrentIndex =
            Queue[QueuePosition++];

        const FWFCCell& CurrentCell =
            Cells[CurrentIndex];

        for (const EChunkConnectionDirection Direction : Directions)
        {
            const FIntPoint NeighborCoordinate =
                CurrentCell.Coordinate +
                DirectionToOffset(Direction);

            if (!IsInsideGrid(NeighborCoordinate))
            {
                continue;
            }

            const int32 NeighborIndex =
                CoordinateToIndex(NeighborCoordinate);

            if (!Cells.IsValidIndex(NeighborIndex))
            {
                continue;
            }

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

    const bool bConnected =
        Visited.Num() == Cells.Num();

    if (!bConnected)
    {
        UE_LOG(
            LogTemp,
            Verbose,
            TEXT(
                "[WFC CONNECTIVITY] Partial solution would "
                "disconnect grid. PotentialReachable=%d/%d"
            ),
            Visited.Num(),
            Cells.Num()
        );
    }

    return bConnected;
}

bool FWFCLevelSolver::SolveRecursive()
{
    /*
     * First make sure the current partial solution
     * has not already become globally disconnected.
     */
    if (!CanStillBecomeFullyConnected())
    {
        return false;
    }

    const int32 CellIndex =
        FindLowestEntropyCell();

    /*
     * No unresolved cells remain.
     */
    if (CellIndex == INDEX_NONE)
    {
        return ValidateSolution();
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
     * We are going to try states one by one.
     *
     * Randomize the order using our seeded RandomStream
     * so generation remains deterministic for a seed.
     */
    TArray<FWFCState> ShuffledStates =
        CandidateStates;

    for (int32 Index = ShuffledStates.Num() - 1;
        Index > 0;
        --Index)
    {
        const int32 SwapIndex =
            RandomStream.RandRange(0, Index);

        ShuffledStates.Swap(
            Index,
            SwapIndex
        );
    }

    for (const FWFCState& CandidateState : ShuffledStates)
    {
        /*
         * Save the complete WFC state before trying
         * this candidate.
         */
        const TArray<FWFCCell> SavedCells =
            Cells;

        /*
         * Force this cell to the candidate state.
         */
        Cells[CellIndex].PossibleStates.Reset();
        Cells[CellIndex].PossibleStates.Add(
            CandidateState
        );

        UE_LOG(
            LogTemp,
            Verbose,
            TEXT(
                "[WFC BACKTRACK] Trying Cell(%d,%d) "
                "Tile=%s Rot=%d"
            ),
            Cells[CellIndex].Coordinate.X,
            Cells[CellIndex].Coordinate.Y,
            CandidateState.Definition
            ? *CandidateState.Definition->ChunkId.ToString()
            : TEXT("NULL"),
            static_cast<int32>(
                CandidateState.Rotation
                ) * 90
        );

        /*
         * First propagate ordinary WFC constraints.
         */
        if (PropagateFrom(CellIndex))
        {
            /*
             * Then make sure we have not cut the
             * potential connectivity graph in half.
             */
            if (CanStillBecomeFullyConnected())
            {
                /*
                 * Continue solving from this state.
                 */
                if (SolveRecursive())
                {
                    return true;
                }
            }
        }

        /*
         * This choice eventually caused either:
         *
         * - a normal WFC contradiction
         * - impossible global connectivity
         * - a contradiction deeper in the search
         *
         * Restore the exact previous state and try
         * another candidate.
         */
        Cells = SavedCells;

        UE_LOG(
            LogTemp,
            Verbose,
            TEXT(
                "[WFC BACKTRACK] Rejected candidate at "
                "Cell(%d,%d). Trying another state."
            ),
            Cells[CellIndex].Coordinate.X,
            Cells[CellIndex].Coordinate.Y
        );
    }

    if (CellIndex == INDEX_NONE)
    {
        if (!ValidateSolution())
        {
            return false;
        }

        return ValidateConnectivity();
    }

    /*
     * Every possible state for this cell failed.
     * Let the previous recursion level backtrack.
     */
    return false;
}