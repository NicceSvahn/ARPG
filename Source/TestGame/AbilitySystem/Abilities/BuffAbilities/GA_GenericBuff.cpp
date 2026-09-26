#include "GA_GenericBuff.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UGA_GenericBuff::UGA_GenericBuff()
{
    bRequiresTargetData = false;
}

void UGA_GenericBuff::ActivateAbility(
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
        !BuffEffect)
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

    if (ActorInfo->IsNetAuthority())
    {
        FGameplayEffectContextHandle EffectContext =
            ASC->MakeEffectContext();

        EffectContext.AddSourceObject(
            ActorInfo->AvatarActor.Get()
        );

        FGameplayEffectSpecHandle EffectSpec =
            ASC->MakeOutgoingSpec(
                BuffEffect,
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
    }

    EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        true,
        false
    );
}
