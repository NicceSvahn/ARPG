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