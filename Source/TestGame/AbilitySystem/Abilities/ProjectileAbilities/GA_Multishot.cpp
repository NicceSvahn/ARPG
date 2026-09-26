#include "GA_Multishot.h"

UGA_Multishot::UGA_Multishot()
{
    DamageData.BaseDamage = 15.0f;
    DamageData.WeaponDamageCoefficient = 0.75f;
    DamageData.DexterityCoefficient = 0.35f;
    DamageData.DamageType = EAbilityDamageType::Physical;
}

void UGA_Multishot::SpawnProjectiles(
    const FProjectileAbilityContext& ProjectileContext)
{
    const float StartAngle =
        -SpreadAngle * 0.5f;

    const float AngleStep =
        ProjectileCount > 1
        ? SpreadAngle /
        static_cast<float>(ProjectileCount - 1)
        : 0.0f;

    for (int32 Index = 0;
        Index < ProjectileCount;
        ++Index)
    {
        const float Angle =
            StartAngle +
            AngleStep *
            static_cast<float>(Index);

        const FVector ProjectileDirection =
            ProjectileContext.BaseDirection
            .RotateAngleAxis(
                Angle,
                FVector::UpVector
            )
            .GetSafeNormal();

        const FGameplayEffectSpecHandle DamageSpec =
            CreateProjectileDamageSpec(
                ProjectileContext.ASC,
                ProjectileContext.Character,
                DamageData
            );

        if (!DamageSpec.IsValid())
        {
            continue;
        }

        SpawnProjectile(
            ProjectileContext.Character,
            ProjectileContext.ASC,
            ProjectileContext.SpawnLocation,
            ProjectileDirection,
            DamageSpec
        );
    }
}