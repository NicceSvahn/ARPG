#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TestGameGameState.generated.h"

USTRUCT(BlueprintType)
struct FEnemyKillProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 Total = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Dead = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
    FOnEnemyKillProgressChanged
);

UENUM(BlueprintType)
enum class ELevelState : uint8
{
    Waiting,
    EncounterActive,
    Intermission,
    Completed,
    Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnLevelStateChanged,
    ELevelState,
    NewState
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnEncounterIndexChanged,
    int32,
    NewEncounterIndex
);

UCLASS()
class TESTGAME_API ATestGameGameState
    : public AGameState
{
    GENERATED_BODY()

public:
    ATestGameGameState();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UFUNCTION(BlueprintPure)
    ELevelState GetLevelState() const
    {
        return LevelState;
    }

    UFUNCTION(BlueprintPure)
    int32 GetCurrentEncounterIndex() const
    {
        return CurrentEncounterIndex;
    }

    void SetLevelState(ELevelState NewState);
    void SetCurrentEncounterIndex(int32 NewIndex);

    UPROPERTY(BlueprintAssignable)
    FOnLevelStateChanged OnLevelStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnEncounterIndexChanged OnEncounterIndexChanged;

    //Procedural map killprogress
    void SetEnemyKillProgress(int32 Total, int32 Dead);

    UFUNCTION(BlueprintPure, Category = "Enemies")
    float GetEnemyProgressToGoal() const;

    UFUNCTION(BlueprintPure, Category = "Enemies")
    int32 GetRequiredEnemyKills() const;

    UFUNCTION(BlueprintPure, Category = "Enemies")
    FEnemyKillProgress GetEnemyKillProgress() const
    {
        return EnemyKillProgress;
    }

    UPROPERTY(BlueprintAssignable, Category = "Enemies")
    FOnEnemyKillProgressChanged OnEnemyKillProgressChanged;

private:
    UPROPERTY(
        ReplicatedUsing = OnRep_LevelState,
        VisibleInstanceOnly
    )
    ELevelState LevelState =
        ELevelState::Waiting;

    UPROPERTY(
        ReplicatedUsing = OnRep_CurrentEncounterIndex,
        VisibleInstanceOnly
    )
    int32 CurrentEncounterIndex =
        INDEX_NONE;

    UFUNCTION()
    void OnRep_LevelState();

    UFUNCTION()
    void OnRep_CurrentEncounterIndex();

    //Procedural map killprogress
    UPROPERTY(ReplicatedUsing = OnRep_EnemyKillProgress)
    FEnemyKillProgress EnemyKillProgress;

    UFUNCTION()
    void OnRep_EnemyKillProgress();
};