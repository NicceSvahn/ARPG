#pragma once

#include "CoreMinimal.h"
#include "WFCLevelTypes.h"

class ULevelChunkDefinition;

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
    // Solver lifecycle
    bool Initialize(
        int32 Width,
        int32 Height,
        const TArray<ULevelChunkDefinition*>& Definitions
    );

    bool RunCollapse();
    bool SolveRecursive();

    // WFC
    int32 FindLowestEntropyCell();

    bool PropagateFrom(int32 CellIndex);

    bool AreStatesCompatible(
        const FWFCState& A,
        const FWFCState& B,
        EChunkConnectionDirection DirectionFromA
    ) const;

    // Constraints
    bool ApplyBoundaryConstraints();

    bool CanStillBecomeFullyConnected() const;

    bool CanPotentiallyConnect(
        const FWFCCell& A,
        const FWFCCell& B,
        EChunkConnectionDirection DirectionFromA
    ) const;

    // Validation
    bool ValidateSolution() const;
    bool ValidateConnectivity() const;

    // Grid helpers
    bool IsInsideGrid(const FIntPoint& Coordinate) const;

    int32 CoordinateToIndex(
        const FIntPoint& Coordinate
    ) const;

    static FIntPoint DirectionToOffset(
        EChunkConnectionDirection Direction
    );

    // Debug helpers
    static const TCHAR* EdgeToString(
        EChunkEdgeType Edge
    );

    static const TCHAR* DirectionToString(
        EChunkConnectionDirection Direction
    );

private:
    int32 GridWidth = 0;
    int32 GridHeight = 0;

    TArray<FWFCCell> Cells;

    FRandomStream RandomStream;
};