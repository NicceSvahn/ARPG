#include "GA_Fireball.h"

#include "../../TestGameAbilitySystemComponent.h"
#include "../../../Characters/GenericCharacter.h"

UGA_Fireball::UGA_Fireball()
{
    DamageData.BaseDamage = 25.0f;
    DamageData.IntellectCoefficient = 0.30f;
    DamageData.SpellPowerCoefficient = 0.80f;
    DamageData.DamageType = EAbilityDamageType::Magic;

    BurnDamageData.BaseDamage = 2.0f;
    BurnDamageData.IntellectCoefficient = 0.05f;
    BurnDamageData.SpellPowerCoefficient = 0.10f;
    BurnDamageData.DamageType = EAbilityDamageType::Magic;
}

void UGA_Fireball::SpawnProjectiles(
    const FProjectileAbilityContext& ProjectileContext)
{
    const FGameplayEffectSpecHandle DamageSpec =
        CreateProjectileDamageSpec(
            ProjectileContext.ASC,
            ProjectileContext.Character,
            DamageData
        );

    if (!DamageSpec.IsValid())
    {
        return;
    }

    FGameplayEffectSpecHandle BurnSpec;

    if (BurnEffect)
    {
        BurnSpec = CreateDamageSpec(
            ProjectileContext.ASC,
            BurnEffect,
            BurnDamageData
        );
    }

    SpawnProjectile(
        ProjectileContext.Character,
        ProjectileContext.ASC,
        ProjectileContext.SpawnLocation,
        ProjectileContext.BaseDirection,
        DamageSpec,
        BurnSpec
    );
}
