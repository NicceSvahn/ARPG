#include "GA_Multishot.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "../../Characters/GenericCharacter.h"
#include "../../AbilitySystem/Abilities/GenericProjectile.h"
#include "../../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../../AbilitySystem/AbilityInputContext.h"


UGA_Multishot::UGA_Multishot()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


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

    AGenericCharacter* Character =
        GetGenericCharacter();

    UTestGameAbilitySystemComponent* ASC =
        Cast<UTestGameAbilitySystemComponent>(
            GetAbilitySystemComponentFromActorInfo()
        );

    if (!Character ||
        !ASC ||
        !ProjectileClass ||
        !DamageEffect)
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

    const FAbilityInputContext& Context =
        ASC->GetAbilityInputContext();

    const FHitResult& CursorHit =
        Context.HitResult;

    if (!CursorHit.bBlockingHit)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MULTISHOT: No valid cursor hit")
        );

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    const FVector TargetLocation =
        Context.HitLocation;

    // Face target horizontally.
    FVector CharacterAimDirection =
        TargetLocation -
        Character->GetActorLocation();

    CharacterAimDirection.Z = 0.0f;

    if (CharacterAimDirection.IsNearlyZero())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MULTISHOT: Invalid character aim direction")
        );

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    Character->SetActorRotation(
        CharacterAimDirection.Rotation()
    );

    // Get muzzle AFTER rotation.
    FVector SpawnLocation =
        Character->GetActorLocation();

    if (Character->GetMesh() &&
        Character->GetMesh()->DoesSocketExist(
            MuzzleSocketName))
    {
        SpawnLocation =
            Character->GetMesh()->GetSocketLocation(
                MuzzleSocketName
            );
    }

    // Small safety offset so projectiles spawn
    // slightly in front of the character.
    SpawnLocation +=
        Character->GetActorForwardVector() * 30.0f;

    const FVector BaseDirection =
        (TargetLocation - SpawnLocation)
        .GetSafeNormal2D();

    if (BaseDirection.IsNearlyZero())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("MULTISHOT: Invalid projectile direction")
        );

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    // Commit cooldown/cost before firing.
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

    if (CastMontage)
    {
        Character->PlayAnimMontage(
            CastMontage
        );
    }

    // Example:
    // ProjectileCount = 3
    // SpreadAngle = 30
    //
    // Results in:
    // -15, 0, +15 degrees.
    const float StartAngle =
        -SpreadAngle * 0.5f;

    const float AngleStep =
        ProjectileCount > 1
        ? SpreadAngle /
        static_cast<float>(
            ProjectileCount - 1)
        : 0.0f;

    if (Character->HasAuthority())
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
                BaseDirection
                .RotateAngleAxis(
                    Angle,
                    FVector::UpVector
                )
                .GetSafeNormal2D();

            if (ProjectileDirection.IsNearlyZero())
            {
                continue;
            }

            /*
             * IMPORTANT:
             * Each projectile gets its own EffectSpec.
             *
             * GenericProjectile adds its own HitResult
             * to the effect context, so the projectiles
             * must not share one mutable spec.
             */
            FGameplayEffectContextHandle EffectContext =
                ASC->MakeEffectContext();

            EffectContext.AddSourceObject(
                Character
            );

            FGameplayEffectSpecHandle DamageSpec =
                ASC->MakeOutgoingSpec(
                    DamageEffect,
                    GetAbilityLevel(),
                    EffectContext
                );

            if (!DamageSpec.IsValid())
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("MULTISHOT: Failed to create damage spec")
                );

                continue;
            }

            const FGameplayTag DamageTag =
                FGameplayTag::RequestGameplayTag(
                    TEXT("Data.Damage")
                );

            DamageSpec.Data
                ->SetSetByCallerMagnitude(
                    DamageTag,
                    MultishotDamage
                );

            const FTransform SpawnTransform(
                ProjectileDirection.Rotation(),
                SpawnLocation
            );

            AGenericProjectile* Projectile =
                GetWorld()
                ->SpawnActorDeferred<
                AGenericProjectile>(
                    ProjectileClass,
                    SpawnTransform,
                    Character,
                    Character,
                    ESpawnActorCollisionHandlingMethod::
                    AlwaysSpawn
                );

            if (!Projectile)
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("MULTISHOT: Failed to spawn projectile %d"),
                    Index
                );

                continue;
            }

            // Initialize BEFORE FinishSpawning.
            Projectile->InitializeProjectile(
                ASC,
                DamageSpec,
                ProjectileDirection
            );

            Projectile->FinishSpawning(
                SpawnTransform
            );

            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "MULTISHOT: Projectile %d Angle=%.1f Direction=%s"
                ),
                Index,
                Angle,
                *ProjectileDirection.ToString()
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