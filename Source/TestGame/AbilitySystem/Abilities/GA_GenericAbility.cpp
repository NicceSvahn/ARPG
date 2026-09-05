#include "GA_GenericAbility.h"

#include "../../Characters/GenericCharacter.h"

UGA_GenericAbility::UGA_GenericAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AGenericCharacter* UGA_GenericAbility::GetGenericCharacter() const
{
    return Cast<AGenericCharacter>(GetAvatarActorFromActorInfo());
}