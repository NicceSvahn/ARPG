#include "GA_Fireball.h"

void UGA_Fireball::SpawnProjectiles(
    const FProjectileAbilityContext& ProjectileContext)
{
    const FGameplayEffectSpecHandle DamageSpec =
        CreateProjectileDamageSpec(
            ProjectileContext.ASC,
            ProjectileContext.Character,
            FireballDamage
        );

    if (!DamageSpec.IsValid())
    {
        return;
    }

    SpawnProjectile(
        ProjectileContext.Character,
        ProjectileContext.ASC,
        ProjectileContext.SpawnLocation,
        ProjectileContext.BaseDirection,
        DamageSpec
    );
}