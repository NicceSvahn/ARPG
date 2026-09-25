#include "GA_GenericMovement.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UGA_GenericMovement::UGA_GenericMovement()
{
    bRequiresTargetData = false;
}

void UGA_GenericMovement::ActivateAbility(
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
        !MovementEffect)
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
            MovementEffect,
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
