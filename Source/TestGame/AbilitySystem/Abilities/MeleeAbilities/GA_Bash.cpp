#include "GA_Bash.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "../../../Characters/GenericCharacter.h"
#include "../../../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../../../AbilitySystem/AbilityInputContext.h"

UGA_Bash::UGA_Bash()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;

    bRequiresTargetData = true;
}

void UGA_Bash::PerformBash()
{
    AGenericCharacter* Character =
        GetGenericCharacter();

    if (!Character ||
        !IsValid(CurrentTargetActor))
    {
        EndAbility(
            CurrentSpecHandle,
            CurrentActorInfo,
            CurrentActivationInfo,
            true,
            true
        );

        return;
    }

    if (!CommitAbility(
        CurrentSpecHandle,
        CurrentActorInfo,
        CurrentActivationInfo))
    {
        EndAbility(
            CurrentSpecHandle,
            CurrentActorInfo,
            CurrentActivationInfo,
            true,
            true
        );

        return;
    }

    FVector Direction =
        CurrentTargetActor->GetActorLocation() -
        Character->GetActorLocation();

    Direction.Z = 0.0f;

    if (!Direction.IsNearlyZero())
    {
        Character->SetActorRotation(
            Direction.Rotation()
        );
    }

    if (BashMontage)
    {
        Character->PlayAnimMontage(
            BashMontage
        );
    }

    ApplyBashDamage();

    EndAbility(
        CurrentSpecHandle,
        CurrentActorInfo,
        CurrentActivationInfo,
        true,
        false
    );
}

void UGA_Bash::ApplyBashDamage()
{
    ApplyDamageToTarget(
        CurrentTargetActor,
        DamageEffect,
        BashDamage
    );
}

void UGA_Bash::OnTargetDataReady(
    const FGameplayAbilityTargetDataHandle& Data)
{
    if (Data.Num() <= 0)
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    const FGameplayAbilityTargetData* TargetData =
        Data.Get(0);

    if (!TargetData)
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    AActor* TargetActor = nullptr;

    if (const FHitResult* HitResult =
        TargetData->GetHitResult())
    {
        TargetActor = HitResult->GetActor();
    }
    else
    {
        const TArray<TWeakObjectPtr<AActor>> TargetActors =
            TargetData->GetActors();

        if (!TargetActors.IsEmpty() &&
            TargetActors[0].IsValid())
        {
            TargetActor = TargetActors[0].Get();
        }
    }

    if (!IsValid(TargetActor))
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    CurrentTargetActor = TargetActor;

    PerformBash();
}