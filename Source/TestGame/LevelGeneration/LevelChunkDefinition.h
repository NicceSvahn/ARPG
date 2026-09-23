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
    North   UMETA(DisplayName = "North"),
    East    UMETA(DisplayName = "East"),
    South   UMETA(DisplayName = "South"),
    West    UMETA(DisplayName = "West")
};

UCLASS(BlueprintType)
class TESTGAME_API ULevelChunkDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Chunk"
    )
    FName ChunkId;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Chunk"
    )
    ELevelChunkType ChunkType = ELevelChunkType::Traversal;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Grid",
        meta = (ClampMin = "1")
    )
    FIntPoint GridSize = FIntPoint(1, 1);

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Generation"
    )
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
        Category = "Generation",
        meta = (ClampMin = "0.0")
    )
    float GenerationWeight = 1.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Connections"
    )
    TSet<EChunkConnectionDirection> Connections;

    UFUNCTION(
        BlueprintPure,
        Category = "Connections"
    )
    bool HasConnection(
        EChunkConnectionDirection Direction
    ) const;

    UFUNCTION(
        BlueprintPure,
        Category = "Connections"
    )
    static EChunkConnectionDirection GetOppositeDirection(
        EChunkConnectionDirection Direction
    );

    UFUNCTION(
        BlueprintPure,
        Category = "Connections"
    )
    static EChunkConnectionDirection GetRotatedDirection(
        EChunkConnectionDirection Direction,
        EChunkRotation Rotation
    );

    UFUNCTION(
        BlueprintPure,
        Category = "Connections"
    )
    TSet<EChunkConnectionDirection> GetConnectionsForRotation(
        EChunkRotation Rotation
    ) const;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Chunk"
    )
    TSoftObjectPtr<UWorld> ChunkLevel;
};