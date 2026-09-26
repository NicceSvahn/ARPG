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

public:
    UGA_Fireball();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    TObjectPtr<UAnimMontage> CastMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Fireball|Burn")
    TSubclassOf<UGameplayEffect> BurnEffect;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Fireball|Burn"
    )
    FAbilityDamageData BurnDamageData;

    virtual void SpawnProjectiles(
        const FProjectileAbilityContext& ProjectileContext
    ) override;
};

