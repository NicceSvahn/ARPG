#include "GA_Fireball.h"

#include "../../../Characters/GenericCharacter.h"
#include "../../TestGameAbilitySystemComponent.h"
#include "GA_ProjectileAbility.h"

void UGA_Fireball::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        TriggerEventData
    );

    FProjectileAbilityContext ProjectileContext;

    if (!PrepareProjectileAbility(
        ProjectileContext))
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    if (!CommitAbility(
        Handle,
        ActorInfo,
        ActivationInfo))
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    if (ProjectileContext.Character->HasAuthority())
    {
        const FGameplayEffectSpecHandle DamageSpec =
            CreateProjectileDamageSpec(
                ProjectileContext.ASC,
                ProjectileContext.Character,
                FireballDamage
            );

        if (DamageSpec.IsValid())
        {
            SpawnProjectile(
                ProjectileContext.Character,
                ProjectileContext.ASC,
                ProjectileContext.SpawnLocation,
                ProjectileContext.BaseDirection,
                DamageSpec
            );
        }
    }

    EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        true,
        false
    );
}