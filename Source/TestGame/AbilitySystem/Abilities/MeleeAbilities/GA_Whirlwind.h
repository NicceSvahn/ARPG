#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"

#include "GA_Whirlwind.generated.h"


class AGenericCharacter;
class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitInputRelease;
class UGameplayEffect;
class UNiagaraSystem;
class UNiagaraComponent;


UCLASS()
class TESTGAME_API UGA_Whirlwind
    : public UGA_GenericAbility
{
    GENERATED_BODY()

public:
    UGA_Whirlwind();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;

    // ANIMATION
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|Animation"
    )
    TObjectPtr<UAnimMontage> WhirlwindMontage = nullptr;

    // VFX
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|VFX"
    )
    TObjectPtr<UNiagaraSystem> WhirlwindEffect = nullptr;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|VFX"
    )
    FVector WhirlwindEffectOffset =
        FVector::ZeroVector;

    // AOE
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|AoE",
        meta = (
            ClampMin = "0.0",
            Units = "cm"
            )
    )
    float Radius = 300.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|AoE"
    )
    TSubclassOf<AGenericCharacter> AffectedCharacterClass;

    // DAMAGE
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|Damage"
    )
    TSubclassOf<UGameplayEffect> DamageEffect;

    // CHANNEL
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|Channel",
        meta = (
            ClampMin = "0.05",
            Units = "s"
            )
    )
    float TickInterval = 0.5f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Whirlwind|Channel",
        meta = (ClampMin = "0.0")
    )
    float ResourceCostPerTick = 5.0f;


private:
    // LIFECYCLE
    void StartWhirlwind();

    void StopWhirlwind(
        bool bWasCancelled
    );

    // VFX
    void StartWhirlwindEffect();

    void StopWhirlwindEffect();

    // GAMEPLAY TICK
    void StartGameplayTimer();

    void StopGameplayTimer();

    void PerformGameplayTick();

    // RESOURCE
    bool CanPayTickCost() const;

    bool ApplyTickCost();

    // DAMAGE
    void ApplyWhirlwindDamage();

    // INPUT
    UFUNCTION()
    void HandleInputReleased(
        float TimeHeld
    );

    // MONTAGE
    UFUNCTION()
    void HandleMontageCompleted();

    UFUNCTION()
    void HandleMontageInterrupted();

    UFUNCTION()
    void HandleMontageCancelled();

    // TASKS / STATE
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait>
        MontageTask = nullptr;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitInputRelease>
        InputReleaseTask = nullptr;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent>
        WhirlwindEffectComponent = nullptr;

    FTimerHandle GameplayTickTimer;

    bool bWhirlwindActive = false;
};