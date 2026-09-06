#pragma once

#include "CoreMinimal.h"
#include "GA_GenericAbility.h"
#include "GA_Fireball.generated.h"

class AGenericProjectile;
class UAnimMontage;
class UGameplayEffect;

UCLASS()
class TESTGAME_API UGA_Fireball : public UGA_GenericAbility
{
	GENERATED_BODY()

public:
	UGA_Fireball();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    TSubclassOf<AGenericProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    TSubclassOf<UGameplayEffect> DamageEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    TObjectPtr<UAnimMontage> CastMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    FName MuzzleSocketName = TEXT("FireballSocket");

    UPROPERTY(EditDefaultsOnly, Category = "Fireball")
    float FireballDamage = 25.0f;
};

