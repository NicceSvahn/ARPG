#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "GA_Sprint.generated.h"

class UGameplayEffect;

UCLASS()
class TESTGAME_API UGA_Sprint : public UGA_GenericAbility
{
    GENERATED_BODY()

public:
    UGA_Sprint();

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
        Category = "Sprint"
    )
    TSubclassOf<UGameplayEffect>
        SprintEffectClass;
};