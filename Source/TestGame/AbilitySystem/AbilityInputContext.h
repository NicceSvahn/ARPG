#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilityInputContext.generated.h"

USTRUCT(BlueprintType)
struct FAbilityInputContext
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY()
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY()
    FHitResult HitResult;

    FGameplayAbilityTargetDataHandle MakeTargetData() const
    {
        FGameplayAbilityTargetDataHandle TargetDataHandle;

        // Precise world hit.
        // Typical source: player cursor trace.
        if (HitResult.bBlockingHit)
        {
            FGameplayAbilityTargetData_SingleTargetHit* TargetData =
                new FGameplayAbilityTargetData_SingleTargetHit(
                    HitResult
                );

            TargetDataHandle.Add(TargetData);

            return TargetDataHandle;
        }

        // Direct actor target.
        // Typical source: server-controlled AI.
        if (IsValid(TargetActor))
        {
            FGameplayAbilityTargetData_ActorArray* TargetData =
                new FGameplayAbilityTargetData_ActorArray();

            TargetData->TargetActorArray.Add(
                TargetActor
            );

            TargetDataHandle.Add(TargetData);
        }

        return TargetDataHandle;
    }
};