#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "EncounterManager.generated.h"

class AEnemyCharacter;
class ALevelGenerator;
class UNavigationSystemV1;
struct FGeneratedChunkInstance;

USTRUCT(BlueprintType)
struct FRandomEnemyEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AEnemyCharacter> EnemyClass;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (ClampMin = "0.0")
    )
    float Weight = 1.0f;
};

UCLASS()
class TESTGAME_API AEncounterManager : public AActor
{
    GENERATED_BODY()

public:
    AEncounterManager();

protected:
    virtual void PostInitializeComponents() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

private:
    void HandleMapReady();
    void TryPopulateMap();
    void HandleChunksClearing();

    void SpawnInChunk(
        const FGeneratedChunkInstance& Chunk,
        UNavigationSystemV1* NavSystem
    );

    UPROPERTY(EditInstanceOnly, Category = "Encounters")
    TObjectPtr<ALevelGenerator> LevelGenerator = nullptr;

    UPROPERTY(EditAnywhere, Category = "Encounters")
    TArray<FRandomEnemyEntry> EnemyTypes;

    UPROPERTY(
        EditAnywhere,
        Category = "Encounters",
        meta = (ClampMin = "0")
    )
    int32 MinEnemiesPerCombatChunk = 1;

    UPROPERTY(
        EditAnywhere,
        Category = "Encounters",
        meta = (ClampMin = "0")
    )
    int32 MaxEnemiesPerCombatChunk = 3;

    UPROPERTY(EditAnywhere, Category = "Encounters")
    int32 EncounterSeed = 4217;

    // Keep spawns away from the edge of a chunk.
    UPROPERTY(
        EditAnywhere,
        Category = "Encounters",
        meta = (ClampMin = "0.0")
    )
    float SpawnInset = 250.0f;

    UPROPERTY(
        EditAnywhere,
        Category = "Encounters",
        meta = (ClampMin = "0.0")
    )
    float MinDistanceFromPlayers = 600.0f;

    FRandomStream RandomStream;
    FTimerHandle NavigationRetryTimer;

    int32 NavigationRetryCount = 0;
    bool bMapPopulated = false;

    TArray<TWeakObjectPtr<AEnemyCharacter>> SpawnedEnemies;

    TSubclassOf<AEnemyCharacter> ChooseEnemyClass();
};