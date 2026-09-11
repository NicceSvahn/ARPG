#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#include "AbilityInputContext.h"

#include "TestGameAbilitySystemComponent.generated.h"

class UGameplayAbility;

DECLARE_MULTICAST_DELEGATE(FOnAbilityBarChanged);

UCLASS()
class TESTGAME_API UTestGameAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    const FAbilityInputContext& GetAbilityInputContext() const
    {
        return AbilityInputContext;
    }

    /**
     * Single entry point for both player input and AI decisions.
     * The tag is resolved against the dynamic tags on granted ability specs.
     */
    bool RequestAbility(const FGameplayTag& AbilityTag, const FAbilityInputContext& Context);

    UGameplayAbility* GetAbilityForInputTag(const FGameplayTag& InputTag) const;

    void NotifyAbilityBarChanged();

    FOnAbilityBarChanged OnAbilityBarChanged;

    void SendAbilityEvent(
        const FGameplayTag& EventTag,
        const FAbilityInputContext& Context
    );

    float GetRemainingCooldown(
        const FGameplayTag& CooldownTag
    ) const;

private:
    FGameplayAbilitySpec* FindAbilitySpecForTag(const FGameplayTag& AbilityTag);

    bool CheckTarget(
        bool bRequiresTarget,
        const FAbilityInputContext& Context
    ) const;

    bool CheckRange(
        float MaximumRange,
        const FAbilityInputContext& Context
    ) const;

    bool TryActivateRequestedAbility(
        const FGameplayAbilitySpecHandle& AbilityHandle,
        const FAbilityInputContext& Context
    );

    bool RequestMovement(
        const FGameplayTag& AbilityTag,
        const FAbilityInputContext& Context,
        float MaximumRange
    );

    void OnRequestMovementCompleted(bool bSuccess);

    FAbilityInputContext AbilityInputContext;

    FGameplayTag PendingAbilityTag;
    FAbilityInputContext PendingAbilityContext;

protected:

};