#pragma once

#include "CoreMinimal.h"
#include "GA_ProjectileAbility.h"
#include "GA_Fireball.generated.h"

class AGenericProjectile;
class UAnimMontage;
class UGameplayEffect;

UCLASS()
class TESTGAME_API UGA_Fireball : public UGA_ProjectileAbility
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    TObjectPtr<UAnimMontage> CastMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    float FireballDamage = 25.0f;


    virtual void SpawnProjectiles(
        const FProjectileAbilityContext& ProjectileContext
    ) override;
};

