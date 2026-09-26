#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "../../AbilityRequestPolicy.h"
#include "GA_GenericMelee.generated.h"

class UAnimMontage;
class UGameplayEffect;

UCLASS(Abstract)
class TESTGAME_API UGA_GenericMelee
    : public UGA_GenericAbility
    , public IAbilityRequestPolicy
{
    GENERATED_BODY()

public:
    UGA_GenericMelee();

    virtual bool RequiresTarget() const override
    {
        return true;
    }

    virtual float GetMaximumRange() const override
    {
        return MeleeRange;
    }

protected:
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Melee",
        meta = (ClampMin = "0.0")
    )
    float MeleeRange = 150.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Melee"
    )
    TSubclassOf<UGameplayEffect> DamageEffect;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Melee"
    )
    TObjectPtr<UAnimMontage> AttackMontage = nullptr;

    virtual void OnTargetDataReady(
        const FGameplayAbilityTargetDataHandle& Data
    ) override;

    /**
     * Override this in a concrete melee ability when it needs extra behavior
     * after the common target/commit setup.
     */
    virtual void ExecuteMeleeAbility(AActor* TargetActor);

    /**
     * Override this when a melee ability needs custom hit behavior.
     * The default implementation applies inherited DamageData through DamageEffect.
     */
    virtual void OnMeleeHit(AActor* TargetActor);

private:
    AActor* ExtractTargetActor(
        const FGameplayAbilityTargetDataHandle& Data
    ) const;
};
