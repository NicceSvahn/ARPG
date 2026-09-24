#pragma once

#include "CoreMinimal.h"
#include "WFCLevelTypes.h"
#include "../LevelChunkDefinition.h"

class TESTGAME_API FWFCLevelSolver
{
public:

    bool Solve(
        int32 Width,
        int32 Height,
        const TArray<ULevelChunkDefinition*>& Definitions,
        int32 Seed
    );

    const TArray<FWFCCell>& GetCells() const
    {
        return Cells;
    }

private:

    int32 GridWidth = 0;
    int32 GridHeight = 0;

    TArray<FWFCCell> Cells;

    FRandomStream RandomStream;

    bool Initialize(
        int32 Width,
        int32 Height,
        const TArray<ULevelChunkDefinition*>& Definitions
    );

    bool RunCollapse();

    int32 FindLowestEntropyCell();

    bool CollapseCell(int32 CellIndex);

    bool PropagateFrom(int32 CellIndex);

    bool AreStatesCompatible(
        const FWFCState& A,
        const FWFCState& B,
        EChunkConnectionDirection DirectionFromA
    ) const;

    bool HasCompatibleNeighborState(
        const FWFCState& State,
        const FWFCCell& Neighbor,
        EChunkConnectionDirection DirectionFromState
    ) const;

    bool IsInsideGrid(
        const FIntPoint& Coordinate
    ) const;

    int32 CoordinateToIndex(
        const FIntPoint& Coordinate
    ) const;

    static FIntPoint DirectionToOffset(
        EChunkConnectionDirection Direction
    );

    bool ValidateSolution() const;

    bool ValidateConnectivity() const;

    static const TCHAR* EdgeToString(
        EChunkEdgeType Edge
    );

    static const TCHAR* DirectionToString(
        EChunkConnectionDirection Direction
    );

    bool ApplyBoundaryConstraints();

    bool CanStillBecomeFullyConnected() const;

    bool CanPotentiallyConnect(
        const FWFCCell& A,
        const FWFCCell& B,
        EChunkConnectionDirection DirectionFromA
    ) const;

    bool SolveRecursive();
};