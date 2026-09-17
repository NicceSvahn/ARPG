#include "GA_Sprint.h"

#include "../../TestGameAbilitySystemComponent.h"
#include "GameplayEffect.h"

UGA_Sprint::UGA_Sprint()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;

    bRequiresTargetData = false;
}

void UGA_Sprint::ActivateAbility(
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

    if (!ActorInfo)
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    if (!ActorInfo->AbilitySystemComponent.IsValid())
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    if (!SprintEffectClass)
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    if (!CommitAbility(
        Handle,
        ActorInfo,
        ActivationInfo))
    {

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    UAbilitySystemComponent* ASC =
        ActorInfo->AbilitySystemComponent.Get();

    FGameplayEffectContextHandle EffectContext =
        ASC->MakeEffectContext();

    EffectContext.AddSourceObject(
        ActorInfo->AvatarActor.Get()
    );

    FGameplayEffectSpecHandle EffectSpec =
        ASC->MakeOutgoingSpec(
            SprintEffectClass,
            GetAbilityLevel(),
            EffectContext
        );

    if (!EffectSpec.IsValid())
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    ASC->ApplyGameplayEffectSpecToSelf(
        *EffectSpec.Data.Get()
    );

    EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        true,
        false
    );
}