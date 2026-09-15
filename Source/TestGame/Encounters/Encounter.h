#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Encounters/EncounterTypes.h"
#include "Encounter.generated.h"

class AEnemySpawner;
class AEnemyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
    FOnEncounterCompleted
);

UCLASS()
class TESTGAME_API AEncounter : public AActor
{
    GENERATED_BODY()

public:
    AEncounter();

    UFUNCTION(BlueprintCallable)
    void StartEncounter();

    UFUNCTION(BlueprintPure)
    EEncounterState GetEncounterState() const
    {
        return EncounterState;
    }

    UFUNCTION(BlueprintPure)
    int32 GetAliveEnemyCount() const;

    UPROPERTY(BlueprintAssignable)
    FOnEncounterCompleted OnEncounterCompleted;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadOnly,
        Category = "Encounter"
    )
    TArray<TObjectPtr<AEnemySpawner>>
        EnemySpawners;


private:
    UPROPERTY()
    TArray<TObjectPtr<AActor>>
        EncounterEnemies;

    UPROPERTY(VisibleInstanceOnly)
    EEncounterState EncounterState =
        EEncounterState::Inactive;

    UFUNCTION()
    void HandleEnemyDestroyed(
        AActor* DestroyedActor
    );

    void RegisterEnemy(
        AActor* Enemy
    );

    void CheckEncounterComplete();
};