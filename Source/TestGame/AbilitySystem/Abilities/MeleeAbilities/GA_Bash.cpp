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
    if (!IsValid(CurrentTargetActor) ||
        !DamageEffect)
    {
        return;
    }

    UAbilitySystemComponent* SourceASC =
        GetAbilitySystemComponentFromActorInfo();

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::
        GetAbilitySystemComponent(
            CurrentTargetActor
        );

    if (!SourceASC ||
        !TargetASC)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext =
        SourceASC->MakeEffectContext();

    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle Spec =
        SourceASC->MakeOutgoingSpec(
            DamageEffect,
            1.0f,
            EffectContext
        );

    if (!Spec.IsValid())
    {
        return;
    }

    SourceASC->ApplyGameplayEffectSpecToTarget(
        *Spec.Data.Get(),
        TargetASC
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

    const TArray<TWeakObjectPtr<AActor>> TargetActors =
        Data.Get(0)->GetActors();

    if (TargetActors.IsEmpty() ||
        !TargetActors[0].IsValid())
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

    CurrentTargetActor =
        TargetActors[0].Get();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[BASH TARGET READY] User=%s | Target=%s | Authority=%s"
        ),
        *GetNameSafe(GetAvatarActorFromActorInfo()),
        *GetNameSafe(CurrentTargetActor),
        GetCurrentActorInfo()->IsNetAuthority()
        ? TEXT("TRUE")
        : TEXT("FALSE")
    );

    PerformBash();
}