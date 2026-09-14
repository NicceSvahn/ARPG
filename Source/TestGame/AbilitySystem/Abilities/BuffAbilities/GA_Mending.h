#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "../../AbilityRequestPolicy.h"
#include "GA_Mending.generated.h"


UCLASS()
class TESTGAME_API UGA_Mending : public UGA_GenericAbility
{
	GENERATED_BODY()

public:
    UGA_Mending();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Healing")
    TSubclassOf<UGameplayEffect>MendingEffect;

};
