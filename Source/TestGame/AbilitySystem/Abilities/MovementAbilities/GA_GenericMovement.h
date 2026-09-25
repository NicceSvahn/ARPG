#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "GA_GenericMovement.generated.h"

class UGameplayEffect;

UCLASS(Abstract)
class TESTGAME_API UGA_GenericMovement : public UGA_GenericAbility
{
    GENERATED_BODY()

public:
    UGA_GenericMovement();

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
        Category = "Ability|Movement"
    )
    TSubclassOf<UGameplayEffect> MovementEffect;
};
