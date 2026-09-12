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
}


void UGA_Bash::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        TriggerEventData
    );

    UTestGameAbilitySystemComponent* ASC =
        Cast<UTestGameAbilitySystemComponent>(
            GetAbilitySystemComponentFromActorInfo()
        );

    if (!ASC)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("BASH: Invalid TestGame ASC")
        );

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    // This context is populated by AbilityInputTagPressed().
    const FAbilityInputContext& InputContext =
        ASC->GetAbilityInputContext();

    CurrentTargetActor =
        InputContext.TargetActor;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BASH: User=%s Target=%s"),
        *GetNameSafe(GetAvatarActorFromActorInfo()),
        *GetNameSafe(CurrentTargetActor)
    );

    PerformBash();
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