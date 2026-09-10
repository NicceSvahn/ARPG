#include "TestGameAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

#include "Abilities/GameplayAbilityTypes.h"

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

        const bool bActivated =
            TryActivateAbility(AbilitySpec.Handle);

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Ability %s activated=%s target=%s"),
            *GetNameSafe(AbilitySpec.Ability),
            bActivated ? TEXT("true") : TEXT("false"),
            *GetNameSafe(Context.TargetActor)
        );

        return;
    }
}

UGameplayAbility*
UTestGameAbilitySystemComponent::GetAbilityForInputTag(
    const FGameplayTag& InputTag) const
{
    if (!InputTag.IsValid())
    {
        return nullptr;
    }

    for (const FGameplayAbilitySpec& AbilitySpec :
        GetActivatableAbilities())
    {
        if (AbilitySpec
            .GetDynamicSpecSourceTags()
            .HasTagExact(InputTag))
        {
            return AbilitySpec.Ability;
        }
    }

    return nullptr;
}

void UTestGameAbilitySystemComponent::NotifyAbilityBarChanged()
{
    OnAbilityBarChanged.Broadcast();
}

void UTestGameAbilitySystemComponent::SendAbilityEvent(
    const FGameplayTag& EventTag,
    const FAbilityInputContext& Context)
{
    if (!EventTag.IsValid())
    {
        return;
    }

    AActor* CurrentAvatarActor = GetAvatarActor();

    if (!IsValid(CurrentAvatarActor))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("SendAbilityEvent: ASC has no valid avatar actor")
        );

        return;
    }

    FGameplayEventData EventData;
    EventData.Instigator = GetAvatarActor();
    EventData.Target = Context.TargetActor;
    //EventData.ContextHandle = MakeEffectContext();

    HandleGameplayEvent(
        EventTag,
        &EventData
    );
}

float UTestGameAbilitySystemComponent::GetRemainingCooldown(
    const FGameplayTag& CooldownTag) const
{
    if (!CooldownTag.IsValid())
    {
        return 0.0f;
    }

    FGameplayTagContainer Tags;
    Tags.AddTag(CooldownTag);

    const FGameplayEffectQuery Query =
        FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            Tags
        );

    const TArray<float> Times =
        GetActiveEffectsTimeRemaining(Query);

    float LongestRemainingTime = 0.0f;

    for (const float TimeRemaining : Times)
    {
        LongestRemainingTime =
            FMath::Max(
                LongestRemainingTime,
                TimeRemaining
            );
    }

    return LongestRemainingTime;
}