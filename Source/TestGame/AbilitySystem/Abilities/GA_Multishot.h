#pragma once

#include "CoreMinimal.h"
#include "GA_GenericAbility.h"
#include "GA_Multishot.generated.h"

class AGenericProjectile;
class UAnimMontage;
class UGameplayEffect;

UCLASS()
class TESTGAME_API UGA_Multishot : public UGA_GenericAbility
{
    GENERATED_BODY()

public:
    UGA_Multishot();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Multishot")
    TSubclassOf<AGenericProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Multishot")
    TSubclassOf<UGameplayEffect> DamageEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Multishot")
    TObjectPtr<UAnimMontage> CastMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Multishot")
    FName MuzzleSocketName = TEXT("MultishotSocket");

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