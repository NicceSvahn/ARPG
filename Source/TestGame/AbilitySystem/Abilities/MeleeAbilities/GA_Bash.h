#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "../../AbilityRequestPolicy.h"
#include "GA_Bash.generated.h"

class UGameplayEffect;
class UAnimMontage;

UCLASS()
class TESTGAME_API UGA_Bash : public UGA_GenericAbility, public IAbilityRequestPolicy
{
    GENERATED_BODY()

public:
    UGA_Bash();

    virtual bool RequiresTarget() const override { return true; }
    virtual float GetMaximumRange() const override { return BashRange; }

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Bash")
    float BashRange = 150.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Bash")
    float BashResourceCost = 10.0f; 

    UPROPERTY(EditDefaultsOnly, Category = "Bash")
    float BashDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Bash")
    TSubclassOf<UGameplayEffect> DamageEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Bash")
    TObjectPtr<UAnimMontage> BashMontage;

private:
    void PerformBash();

    void ApplyBashDamage();

    UPROPERTY()
    TObjectPtr<AActor> CurrentTargetActor = nullptr;
};