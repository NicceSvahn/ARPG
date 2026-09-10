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

UGameplayAbility* UTestGameAbilitySystemComponent::GetAbilityForInputTag(
    const FGameplayTag& InputTag) const
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("LOOKUP TAG: %s | Ability count: %d"),
        *InputTag.ToString(),
        GetActivatableAbilities().Num()
    );

    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("ABILITY: %s | Tags: %s"),
            *GetNameSafe(AbilitySpec.Ability),
            *AbilitySpec.GetDynamicSpecSourceTags().ToString()
        );

        if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("FOUND: %s"),
                *GetNameSafe(AbilitySpec.Ability)
            );

            return AbilitySpec.Ability;
        }
    }

    return nullptr;
}