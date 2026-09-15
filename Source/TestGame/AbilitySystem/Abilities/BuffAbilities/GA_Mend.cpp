#include "GA_Mend.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "TestGame/Characters/GenericCharacter.h"

UGA_Mend::UGA_Mend()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Mend::ActivateAbility(
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

    if (!MendEffect)
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
            MendEffect,
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