#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "../../AbilityRequestPolicy.h"
#include "GA_Mend.generated.h"


UCLASS()
class TESTGAME_API UGA_Mend : public UGA_GenericAbility
{
	GENERATED_BODY()

public:
    UGA_Mend();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Healing")
    TSubclassOf<UGameplayEffect>MendEffect;

};
