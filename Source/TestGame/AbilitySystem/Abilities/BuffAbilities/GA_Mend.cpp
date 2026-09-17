#include "GA_Mend.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UGA_Mend::UGA_Mend()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;

    bRequiresTargetData = false;
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

    if (!ActorInfo ||
        !ActorInfo->AbilitySystemComponent.IsValid() ||
        !MendEffect)
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

    // Gameplay truth belongs to authority.
    if (ActorInfo->IsNetAuthority())
    {
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

        if (EffectSpec.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(
                *EffectSpec.Data.Get()
            );
        }
    }

    EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        true,
        false
    );
}