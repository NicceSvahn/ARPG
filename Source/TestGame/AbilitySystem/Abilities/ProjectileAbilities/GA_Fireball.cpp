#include "GA_Fireball.h"

#include "../../TestGameAbilitySystemComponent.h"
#include "../../../Characters/GenericCharacter.h"

void UGA_Fireball::SpawnProjectiles(
    const FProjectileAbilityContext& ProjectileContext)
{
    const FGameplayEffectSpecHandle DamageSpec =
        CreateDamageSpec(
            ProjectileContext.ASC,
            DamageEffect,
            FireballDamage
        );

    if (!DamageSpec.IsValid())
    {
        return;
    }

    FGameplayEffectSpecHandle BurnSpec;

    if (BurnEffect)
    {
        FGameplayEffectContextHandle BurnContext =
            ProjectileContext.ASC->MakeEffectContext();

        BurnContext.AddSourceObject(
            ProjectileContext.Character
        );

        BurnSpec =
            ProjectileContext.ASC->MakeOutgoingSpec(
                BurnEffect,
                GetAbilityLevel(),
                BurnContext
            );

        if (BurnSpec.IsValid())
        {
            const FGameplayTag DamageTag =
                FGameplayTag::RequestGameplayTag(
                    TEXT("Data.Damage")
                );

            BurnSpec.Data->SetSetByCallerMagnitude(
                DamageTag,
                BurnDamage
            );
        }
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