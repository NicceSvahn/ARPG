#include "GA_Multishot.h"

#include "../../../Characters/GenericCharacter.h"
#include "../../TestGameAbilitySystemComponent.h"
#include "GA_ProjectileAbility.h"

void UGA_Multishot::ActivateAbility(
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

    const float StartAngle =
        -SpreadAngle * 0.5f;

    const float AngleStep =
        ProjectileCount > 1
        ? SpreadAngle /
        static_cast<float>(
            ProjectileCount - 1
            )
        : 0.0f;

    if (ProjectileContext.Character->HasAuthority())
    {
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
                .GetSafeNormal2D();

            const FGameplayEffectSpecHandle DamageSpec =
                CreateProjectileDamageSpec(
                    ProjectileContext.ASC,
                    ProjectileContext.Character,
                    MultishotDamage
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

    EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        true,
        false
    );
}