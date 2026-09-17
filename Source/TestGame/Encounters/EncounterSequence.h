#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Game/TestGameGameState.h"
#include "EncounterSequence.generated.h"

class AEncounter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
    FOnEncounterSequenceCompleted
);

UCLASS()
class TESTGAME_API AEncounterSequence : public AActor
{
    GENERATED_BODY()

public:
    AEncounterSequence();

    UFUNCTION(BlueprintCallable)
    void StartSequence();

    UFUNCTION(BlueprintPure)
    int32 GetCurrentEncounterNumber() const;

    UFUNCTION(BlueprintPure)
    bool IsSequenceActive() const
    {
        return bSequenceActive;
    }

    UPROPERTY(BlueprintAssignable)
    FOnEncounterSequenceCompleted OnSequenceCompleted;

    UFUNCTION(BlueprintCallable)
    void StopSequence();

protected:
    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadOnly,
        Category = "Encounter Sequence"
    )
    TArray<TObjectPtr<AEncounter>> Encounters;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Encounter Sequence",
        meta = (ClampMin = "0.0")
    )
    float DelayBetweenEncounters = 3.0f;

private:
    int32 CurrentEncounterIndex = INDEX_NONE;

    bool bSequenceActive = false;

    FTimerHandle EncounterDelayTimer;

    void StartCurrentEncounter();

    UFUNCTION()
    void HandleEncounterCompleted();

    void CompleteSequence();

    UFUNCTION()
    void HandleLevelStateChanged(
        ELevelState NewState
    );
};