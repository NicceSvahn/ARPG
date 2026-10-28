#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "../../AbilityRequestPolicy.h"
#include "GA_GenericMelee.generated.h"

class AGenericCharacter;
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

    virtual bool SupportsForceDirectionalAttack() const override
    {
        return true;
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

    // When a normal single-target melee ability is force-cast with Shift,
    // this arc is used to determine whether something was actually hit.
    // Abilities with their own hit query (for example Cleave) override
    // OnMeleeHit and do not use this setting.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Melee|Directional",
        meta = (ClampMin = "0.0", ClampMax = "360.0")
    )
    float DirectionalHitArcDegrees = 90.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Melee|Directional"
    )
    TSubclassOf<AGenericCharacter> DirectionalTargetClass;

    // Animations
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

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;

    bool bMeleeHitTriggered = false;

private:
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask = nullptr;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> HitEventTask = nullptr;

    TWeakObjectPtr<AActor> PendingTargetActor;

    bool bForceDirectionalAttack = false;
    FVector PendingDirectionalAimLocation = FVector::ZeroVector;

    // Melee montages hard-lock locomotion for their duration. We cache the
    // previous movement mode so movement can resume as soon as the montage
    // ends or the ability is cancelled.
    bool bMovementLockedForMontage = false;
    uint8 CachedMovementMode = 0;
    uint8 CachedCustomMovementMode = 0;

    void LockMovementForMontage(
        AGenericCharacter* Character
    );

    void UnlockMovementAfterMontage();

    AActor* ExtractTargetActor(
        const FGameplayAbilityTargetDataHandle& Data
    ) const;

    bool ExtractDirectionalAimLocation(
        const FGameplayAbilityTargetDataHandle& Data,
        FVector& OutAimLocation
    ) const;

    void FaceWorldLocation(
        AGenericCharacter* Character,
        const FVector& WorldLocation
    ) const;

    AActor* FindDirectionalMeleeTarget();

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
