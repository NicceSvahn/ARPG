#pragma once

#include "CoreMinimal.h"
#include "GA_ProjectileAbility.h"
#include "GA_Multishot.generated.h"

class UAnimMontage;

UCLASS()
class TESTGAME_API UGA_Multishot : public UGA_ProjectileAbility
{
    GENERATED_BODY()

protected:

    virtual void SpawnProjectiles(
        const FProjectileAbilityContext& ProjectileContext
    ) override;

    UPROPERTY(EditDefaultsOnly, Category = "Multishot")
    TObjectPtr<UAnimMontage> CastMontage;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Multishot",
        meta = (ClampMin = "3")
    )
    int32 ProjectileCount = 3;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Multishot",
        meta = (ClampMin = "0.0")
    )
    float SpreadAngle = 30.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Multishot")
    float MultishotDamage = 15.0f;
};