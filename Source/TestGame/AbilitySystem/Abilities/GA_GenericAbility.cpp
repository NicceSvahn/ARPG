#include "GA_GenericAbility.h"

#include "../Attributes/ResourceAttributeSet.h"
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

bool UGA_GenericAbility::CheckCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    FGameplayTagContainer* OptionalRelevantTags
) const
{
    if (!Super::CheckCost(
        Handle,
        ActorInfo,
        OptionalRelevantTags))
    {
        return false;
    }

    if (ResourceCost <= 0.0f)
    {
        return true;
    }

    const UAbilitySystemComponent* ASC =
        ActorInfo
        ? ActorInfo->AbilitySystemComponent.Get()
        : nullptr;

    if (!ASC)
    {
        return false;
    }

    const float CurrentResource =
        ASC->GetNumericAttribute(
            UResourceAttributeSet::GetResourceAttribute()
        );

    return CurrentResource >= ResourceCost;
}

void UGA_GenericAbility::ApplyCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo
) const
{
    if (ResourceCost <= 0.0f)
    {
        return;
    }

    UAbilitySystemComponent* ASC =
        ActorInfo
        ? ActorInfo->AbilitySystemComponent.Get()
        : nullptr;

    if (!ASC || !CostGameplayEffectClass)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext =
        ASC->MakeEffectContext();

    EffectContext.AddSourceObject(
        ActorInfo->AvatarActor.Get()
    );

    FGameplayEffectSpecHandle CostSpec =
        ASC->MakeOutgoingSpec(
            CostGameplayEffectClass,
            GetAbilityLevel(),
            EffectContext
        );

    if (!CostSpec.IsValid())
    {
        return;
    }

    const FGameplayTag ResourceCostTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Data.ResourceCost")
        );

    CostSpec.Data->SetSetByCallerMagnitude(
        ResourceCostTag,
        -ResourceCost
    );

    ASC->ApplyGameplayEffectSpecToSelf(
        *CostSpec.Data.Get()
    );
}