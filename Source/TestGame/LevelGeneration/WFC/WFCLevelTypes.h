#pragma once

#include "CoreMinimal.h"
#include "../LevelChunkDefinition.h"
#include "WFCLevelTypes.generated.h"

USTRUCT(BlueprintType)
struct FWFCState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<ULevelChunkDefinition> Definition = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EChunkRotation Rotation = EChunkRotation::Degrees0;

    bool operator==(const FWFCState& Other) const
    {
        return
            Definition == Other.Definition &&
            Rotation == Other.Rotation;
    }
};

USTRUCT()
struct FWFCCell
{
    GENERATED_BODY()

    FIntPoint Coordinate = FIntPoint::ZeroValue;

    TArray<FWFCState> PossibleStates;

    bool IsCollapsed() const
    {
        return PossibleStates.Num() == 1;
    }

    bool IsContradiction() const
    {
        return PossibleStates.IsEmpty();
    }

    int32 GetEntropy() const
    {
        return PossibleStates.Num();
    }
};