#include "GA_GenericMelee.h"

#include "../../../Characters/GenericCharacter.h"

UGA_GenericMelee::UGA_GenericMelee()
{
    bRequiresTargetData = true;
}

AActor* UGA_GenericMelee::ExtractTargetActor(
    const FGameplayAbilityTargetDataHandle& Data) const
{
    if (Data.Num() <= 0)
    {
        return nullptr;
    }

    const FGameplayAbilityTargetData* TargetData =
        Data.Get(0);

    if (!TargetData)
    {
        return nullptr;
    }

    if (const FHitResult* HitResult =
        TargetData->GetHitResult())
    {
        return HitResult->GetActor();
    }

    const TArray<TWeakObjectPtr<AActor>> TargetActors =
        TargetData->GetActors();

    if (!TargetActors.IsEmpty() &&
        TargetActors[0].IsValid())
    {
        return TargetActors[0].Get();
    }

    return nullptr;
}

void UGA_GenericMelee::OnTargetDataReady(
    const FGameplayAbilityTargetDataHandle& Data)
{
    AActor* TargetActor =
        ExtractTargetActor(Data);

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

    ExecuteMeleeAbility(TargetActor);
}

void UGA_GenericMelee::ExecuteMeleeAbility(
    AActor* TargetActor)
{
    AGenericCharacter* Character =
        GetGenericCharacter();

    if (!Character ||
        !IsValid(TargetActor))
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

    if (!CommitAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo()))
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

    FVector Direction =
        TargetActor->GetActorLocation() -
        Character->GetActorLocation();

    Direction.Z = 0.0f;

    if (!Direction.IsNearlyZero())
    {
        Character->SetActorRotation(
            Direction.Rotation()
        );
    }

    if (AttackMontage)
    {
        Character->PlayAnimMontage(
            AttackMontage
        );
    }

    OnMeleeHit(TargetActor);

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true,
        false
    );
}

void UGA_GenericMelee::OnMeleeHit(
    AActor* TargetActor)
{
    ApplyDamageToTarget(
        TargetActor,
        DamageEffect,
        MeleeDamage
    );
}
