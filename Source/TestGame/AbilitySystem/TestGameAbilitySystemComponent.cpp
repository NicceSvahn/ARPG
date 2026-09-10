#include "TestGameAbilitySystemComponent.h"

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
    UE_LOG(
        LogTemp,
        Error,
        TEXT("No granted ability found for tag %s"),
        *InputTag.ToString()
    );
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