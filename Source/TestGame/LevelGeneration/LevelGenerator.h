#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelChunkDefinition.h"
#include "LevelGenerator.generated.h"

class ULevelStreamingDynamic;
class ANavMeshBoundsVolume;

USTRUCT(BlueprintType)
struct FGeneratedChunk
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<ULevelChunkDefinition> Definition = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FIntPoint GridCoordinate = FIntPoint::ZeroValue;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EChunkRotation Rotation = EChunkRotation::Degrees0;
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(
    FOnGeneratedChunkReady,
    ULevelStreamingDynamic*,
    ULevelChunkDefinition*,
    FIntPoint
);

DECLARE_MULTICAST_DELEGATE(FOnGeneratedMapReady);
DECLARE_MULTICAST_DELEGATE(FOnGeneratedChunksClearing);

USTRUCT()
struct FGeneratedChunkInstance
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<ULevelStreamingDynamic> StreamingLevel = nullptr;

    UPROPERTY()
    TObjectPtr<ULevelChunkDefinition> Definition = nullptr;

    FIntPoint GridCoordinate = FIntPoint::ZeroValue;

    bool bReadyBroadcast = false;
};

UCLASS()
class TESTGAME_API ALevelGenerator : public AActor
{
    GENERATED_BODY()

public:
    ALevelGenerator();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Level Generation")
    void GenerateLevel();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Level Generation")
    void SpawnGeneratedChunks();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Level Generation")
    void ClearGeneratedChunks();

    FOnGeneratedChunkReady OnGeneratedChunkReady;
    FOnGeneratedChunksClearing OnGeneratedChunksClearing;

    FOnGeneratedMapReady OnGeneratedMapReady;

    const TArray<FGeneratedChunkInstance>& GetChunkInstances() const
    {
        return GeneratedChunkInstances;
    }

    float GetChunkSize() const
    {
        return ChunkSize;
    }

protected:
    virtual void BeginPlay() override;

private:
    FVector GridToWorldLocation(
        const FIntPoint& GridCoordinate
    ) const;

    static FRotator ChunkRotationToWorldRotation(
        EChunkRotation Rotation
    );

    bool ValidateGeneratedChunks() const;

    void UpdateNavigationBounds();

private:
    UPROPERTY(
        EditAnywhere,
        Category = "Level Generation|WFC",
        meta = (ClampMin = "100.0")
    )
    float ChunkSize = 2000.0f;

    UPROPERTY(
        EditAnywhere,
        Category = "Level Generation|WFC",
        meta = (ClampMin = "1")
    )
    int32 GridWidth = 6;

    UPROPERTY(
        EditAnywhere,
        Category = "Level Generation|WFC",
        meta = (ClampMin = "1")
    )
    int32 GridHeight = 6;

    UPROPERTY(EditAnywhere, Category = "Level Generation|WFC")
    int32 GenerationSeed = 1337;

    UPROPERTY(EditAnywhere, Category = "Level Generation|WFC")
    TArray<TObjectPtr<ULevelChunkDefinition>> WFCTiles;

    UPROPERTY(EditAnywhere, Category = "Level Generation|Runtime")
    bool bGenerateOnBeginPlay = false;

    UPROPERTY(VisibleAnywhere, Category = "Level Generation|Debug")
    TArray<FGeneratedChunk> GeneratedChunks;

    UPROPERTY(Transient)
    TArray<TObjectPtr<ULevelStreamingDynamic>> SpawnedChunkLevels;

    UFUNCTION()
    void HandleChunkLevelShown();

    UPROPERTY(Transient)
    TArray<FGeneratedChunkInstance> GeneratedChunkInstances;

    UPROPERTY(
        EditInstanceOnly,
        Category = "Level Generation|Navigation"
    )
    TObjectPtr<ANavMeshBoundsVolume> NavigationBoundsVolume = nullptr;

    UPROPERTY(
        EditAnywhere,
        Category = "Level Generation|Navigation",
        meta = (ClampMin = "0.0")
    )
    float NavigationMargin = 200.0f;

    UPROPERTY(
        EditAnywhere,
        Category = "Level Generation|Navigation",
        meta = (ClampMin = "100.0")
    )
    float NavigationHeight = 1000.0f;

    UPROPERTY(
        EditAnywhere,
        Category = "Level Generation|Navigation"
    )
    float NavigationCenterZ = 100.0f;

    void TryBroadcastMapReady();

    bool bFinishedSchedulingChunks = false;
    bool bMapReadyBroadcast = false;
};