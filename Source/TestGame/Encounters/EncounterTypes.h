#pragma once

#include "CoreMinimal.h"
#include "EncounterTypes.generated.h"

USTRUCT(BlueprintType)
struct FEncounterEnemyEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> EnemyClass;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        meta = (ClampMin = "1")
    )
    int32 Count = 1;
};

UENUM(BlueprintType)
enum class EEncounterState : uint8
{
    Inactive,
    Active,
    Completed
};