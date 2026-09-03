#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyCharacter;

USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AEnemyCharacter> EnemyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner", meta = (ClampMin = "1"))
    int32 Count = 1;
};

UCLASS()
class TESTGAME_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawner();

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnEnemies();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
    TArray<FEnemySpawnEntry> SpawnEntries;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.0"))
    float SpawnRadius = 300.0f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner")
    TArray<TObjectPtr<AEnemyCharacter>> SpawnedEnemies;

private:
    FVector GetRandomSpawnLocation() const;
};