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

    void AbilityInputTagPressed(const FGameplayTag& InputTag, const FAbilityInputContext& Context);

    UGameplayAbility* GetAbilityForInputTag(const FGameplayTag& InputTag) const;

    void NotifyAbilityBarChanged();

    FOnAbilityBarChanged OnAbilityBarChanged;

private:

    FAbilityInputContext AbilityInputContext;

protected:

};