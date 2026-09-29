#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "../../AbilityRequestPolicy.h"
#include "GA_GenericMelee.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

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

    //Animations
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Melee"
    )
    TObjectPtr<UAnimMontage> AttackMontage = nullptr;

    virtual UAnimMontage* GetAttackMontageForActivation();

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Melee",
        meta = (Categories = "Event.Combat")
    )
    FGameplayTag MeleeHitEventTag;

    // Handling
    virtual void OnTargetDataReady(
        const FGameplayAbilityTargetDataHandle& Data
    ) override;

    virtual void ExecuteMeleeAbility(
        AActor* TargetActor
    );

    virtual void OnMeleeHit(
        AActor* TargetActor
    );

    bool bMeleeHitTriggered = false;

private:
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> HitEventTask = nullptr;

    TWeakObjectPtr<AActor> PendingTargetActor;

    AActor* ExtractTargetActor(
        const FGameplayAbilityTargetDataHandle& Data
    ) const;

    UFUNCTION()
    void HandleMeleeHitEvent(
        FGameplayEventData Payload
    );

    UFUNCTION()
    void HandleMontageCompleted();

    UFUNCTION()
    void HandleMontageInterrupted();

    UFUNCTION()
    void HandleMontageCancelled();

    void FinishMeleeAbility(
        bool bWasCancelled
    );
};