#include "GA_Fireball.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "../../Characters/GenericCharacter.h"
#include "../../AbilitySystem/Abilities/GenericProjectile.h"
#include "../../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../../AbilitySystem/AbilityInputContext.h"


UGA_Fireball::UGA_Fireball()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;
}


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
            TEXT("FIREBALL: No valid cursor hit")
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

    // Face cursor horizontally.
    FVector CharacterAimDirection =
        TargetLocation - Character->GetActorLocation();

    CharacterAimDirection.Z = 0.0f;

    if (!CharacterAimDirection.IsNearlyZero())
    {
        Character->SetActorRotation(
            CharacterAimDirection.Rotation()
        );
    }

    // Get muzzle AFTER rotation.
    FVector SpawnLocation =
        Character->GetActorLocation() +
        Character->GetActorForwardVector() * 100.0f;

    if (Character->GetMesh() &&
        Character->GetMesh()->DoesSocketExist(MuzzleSocketName))
    {
        SpawnLocation =
            Character->GetMesh()->GetSocketLocation(
                MuzzleSocketName
            );
    }

    // Actual projectile aim.
    const FVector ProjectileDirection =
        (TargetLocation - SpawnLocation).GetSafeNormal2D();

    if (CharacterAimDirection.IsNearlyZero())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("FIREBALL: Invalid launch direction"));

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true);

        return;
    }

    Character->SetActorRotation(CharacterAimDirection.Rotation());

    if (ProjectileDirection.IsNearlyZero())
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
        Character->PlayAnimMontage(CastMontage);
    }

    if (Character->HasAuthority())
    {
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

        if (DamageSpec.IsValid())
        {
            const FGameplayTag DamageTag =
                FGameplayTag::RequestGameplayTag(
                    TEXT("Data.Damage")
                );

            DamageSpec.Data->SetSetByCallerMagnitude(
                DamageTag,
                FireballDamage
            );

            const FTransform SpawnTransform(
                ProjectileDirection.Rotation(),
                SpawnLocation
            );

            AGenericProjectile* Projectile =
                GetWorld()->SpawnActorDeferred<
                AGenericProjectile
                >(
                    ProjectileClass,
                    SpawnTransform,
                    Character,
                    Character,
                    ESpawnActorCollisionHandlingMethod::AlwaysSpawn
                );

            if (Projectile)
            {
                Projectile->FinishSpawning(
                    SpawnTransform
                );

                Projectile->InitializeProjectile(
                    ASC,
                    DamageSpec,
                    nullptr,
                    ProjectileDirection
                );
            }
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