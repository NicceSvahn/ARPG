#include "TestGameAbilitySystemComponent.h"

void UTestGameAbilitySystemComponent::AbilityInputTagPressed(
    const FGameplayTag& InputTag,
    const FAbilityInputContext& Context)
{
    if (!InputTag.IsValid())
    {
        return;
    }

    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (!AbilitySpec
            .GetDynamicSpecSourceTags()
            .HasTagExact(InputTag))
        {
            continue;
        }

        AbilityInputContext = Context;

        TryActivateAbility(AbilitySpec.Handle);

        return;
    }
}

UGameplayAbility* UTestGameAbilitySystemComponent::GetAbilityForInputTag(const FGameplayTag& InputTag) const
{
    if (!InputTag.IsValid())
    {
        return nullptr;
    }

    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
        {
            continue;
        }

        if (AbilitySpec.Ability)
        {
            return AbilitySpec.Ability;
        }
    }

    return nullptr;
}