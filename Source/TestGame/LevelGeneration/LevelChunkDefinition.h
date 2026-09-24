#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "LevelChunkDefinition.generated.h"

UENUM(BlueprintType)
enum class ELevelChunkType : uint8
{
    Traversal   UMETA(DisplayName = "Traversal"),
    Combat      UMETA(DisplayName = "Combat"),
    Junction    UMETA(DisplayName = "Junction"),
    DeadEnd     UMETA(DisplayName = "Dead End"),
    Start       UMETA(DisplayName = "Start"),
    End         UMETA(DisplayName = "End")
};

UENUM(BlueprintType)
enum class EChunkRotation : uint8
{
    Degrees0    UMETA(DisplayName = "0"),
    Degrees90   UMETA(DisplayName = "90"),
    Degrees180  UMETA(DisplayName = "180"),
    Degrees270  UMETA(DisplayName = "270")
};

UENUM(BlueprintType)
enum class EChunkConnectionDirection : uint8
{
    North,
    East,
    South,
    West
};

UENUM(BlueprintType)
enum class EChunkEdgeType : uint8
{
    Closed  UMETA(DisplayName = "Closed"),
    Open    UMETA(DisplayName = "Open")
};

USTRUCT(BlueprintType)
struct FChunkEdges
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EChunkEdgeType North = EChunkEdgeType::Closed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EChunkEdgeType East = EChunkEdgeType::Closed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EChunkEdgeType South = EChunkEdgeType::Closed;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EChunkEdgeType West = EChunkEdgeType::Closed;
};

UCLASS(BlueprintType)
class TESTGAME_API ULevelChunkDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk")
    FName ChunkId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk")
    ELevelChunkType ChunkType = ELevelChunkType::Traversal;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk")
    FIntPoint GridSize = FIntPoint(1, 1);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chunk")
    TSoftObjectPtr<UWorld> ChunkLevel;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WFC")
    FChunkEdges Edges;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WFC")
    TArray<EChunkRotation> AllowedRotations =
    {
        EChunkRotation::Degrees0,
        EChunkRotation::Degrees90,
        EChunkRotation::Degrees180,
        EChunkRotation::Degrees270
    };

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "WFC",
        meta = (ClampMin = "0.01")
    )
    float GenerationWeight = 1.0f;

    UFUNCTION(BlueprintPure, Category = "WFC")
    EChunkEdgeType GetEdge(
        EChunkConnectionDirection Direction,
        EChunkRotation Rotation
    ) const;

    static EChunkConnectionDirection GetOppositeDirection(
        EChunkConnectionDirection Direction
    );

    static EChunkConnectionDirection GetRotatedDirection(
        EChunkConnectionDirection Direction,
        EChunkRotation Rotation
    );
};