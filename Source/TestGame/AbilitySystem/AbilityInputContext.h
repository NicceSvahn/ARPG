#pragma once

#include "CoreMinimal.h"
#include "AbilityInputContext.generated.h"

USTRUCT(BlueprintType)
struct FAbilityInputContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FHitResult HitResult;
};