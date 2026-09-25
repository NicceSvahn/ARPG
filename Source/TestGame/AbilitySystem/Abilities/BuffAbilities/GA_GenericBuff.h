#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "GA_GenericBuff.generated.h"

class UGameplayEffect;

UCLASS(Abstract)
class TESTGAME_API UGA_GenericBuff : public UGA_GenericAbility
{
    GENERATED_BODY()

public:
    UGA_GenericBuff();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Buff"
    )
    TSubclassOf<UGameplayEffect> BuffEffect;
};
