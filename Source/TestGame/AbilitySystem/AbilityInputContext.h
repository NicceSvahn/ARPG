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

    /**
     * When true, a melee ability should ignore its actor target and perform
     * a stationary directional attack toward the cursor instead.
     *
     * This value is captured when the ability button is pressed.
     */
    UPROPERTY()
    bool bForceDirectionalAttack = false;

    FGameplayAbilityTargetDataHandle MakeTargetData() const
    {
        FGameplayAbilityTargetDataHandle TargetDataHandle;

        // Forced directional melee attacks deliberately send LOCATION data,
        // not actor target data. This is important for networking: the server
        // can distinguish "attack toward this point" from a normal targeted
        // melee attack without needing to know the client's Shift key state.
        if (bForceDirectionalAttack)
        {
            if (!HitResult.bBlockingHit)
            {
                return TargetDataHandle;
            }

            FGameplayAbilityTargetData_LocationInfo* LocationData =
                new FGameplayAbilityTargetData_LocationInfo();

            LocationData->TargetLocation.LocationType =
                EGameplayAbilityTargetingLocationType::LiteralTransform;

            LocationData->TargetLocation.LiteralTransform =
                FTransform(
                    FRotator::ZeroRotator,
                    HitLocation
                );

            TargetDataHandle.Add(LocationData);

            return TargetDataHandle;
        }

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
