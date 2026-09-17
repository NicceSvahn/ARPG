#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TestGameGameState.generated.h"

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
};