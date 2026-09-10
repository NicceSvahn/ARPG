#include "GA_GenericAbility.h"

#include "../../Characters/GenericCharacter.h"

UGA_GenericAbility::UGA_GenericAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AGenericCharacter* UGA_GenericAbility::GetGenericCharacter() const
{
    return Cast<AGenericCharacter>(GetAvatarActorFromActorInfo());
}

const FGameplayTagContainer*
UGA_GenericAbility::GetCooldownTags() const
{
    CooldownTagContainer.Reset();

    if (CooldownTag.IsValid())
    {
        CooldownTagContainer.AddTag(CooldownTag);
    }

    return &CooldownTagContainer;
}

void UGA_GenericAbility::ApplyCooldown(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo) const
{
    if (!CooldownEffectTemplate ||
        CooldownDuration <= 0.0f ||
        !CooldownTag.IsValid())
    {
        return;
    }

    FGameplayEffectSpecHandle SpecHandle =
        MakeOutgoingGameplayEffectSpec(
            Handle,
            ActorInfo,
            ActivationInfo,
            CooldownEffectTemplate,
            GetAbilityLevel(Handle, ActorInfo)
        );

    if (!SpecHandle.IsValid())
    {
        return;
    }

    SpecHandle.Data->SetDuration(
        CooldownDuration,
        true
    );

    SpecHandle.Data->DynamicGrantedTags.AddTag(
        CooldownTag
    );

    ApplyGameplayEffectSpecToOwner(
        Handle,
        ActorInfo,
        ActivationInfo,
        SpecHandle
    );
}