#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EncounterManager.generated.h"

class AEnemyCharacter;
class ALevelGenerator;
class ULevelChunkDefinition;
class ULevelStreamingDynamic;

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
    void HandleChunkReady(
        ULevelStreamingDynamic* StreamingLevel,
        ULevelChunkDefinition* Definition,
        FIntPoint GridCoordinate
    );

    void HandleChunksClearing();

    UPROPERTY(
        EditInstanceOnly,
        Category = "Encounters"
    )
    TObjectPtr<ALevelGenerator> LevelGenerator = nullptr;

    UPROPERTY(
        EditAnywhere,
        Category = "Encounters"
    )
    TSubclassOf<AEnemyCharacter> EnemyClass;

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

    UPROPERTY(
        EditAnywhere,
        Category = "Encounters"
    )
    int32 EncounterSeed = 4217;

    FRandomStream RandomStream;

    TArray<TWeakObjectPtr<AEnemyCharacter>> SpawnedEnemies;
};