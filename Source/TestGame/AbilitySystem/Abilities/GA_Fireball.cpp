#include "GA_Fireball.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "../../Characters/GenericCharacter.h"
#include "../../AbilitySystem/Abilities/GenericProjectile.h"

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
        TriggerEventData);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("FIREBALL: ActivateAbility called. Authority=%s"),
        GetGenericCharacter() && GetGenericCharacter()->HasAuthority()
        ? TEXT("true")
        : TEXT("false"));

    AGenericCharacter* Character = GetGenericCharacter();

    UAbilitySystemComponent* SourceASC =
        GetAbilitySystemComponentFromActorInfo();

    if (!Character ||
        !SourceASC ||
        !ProjectileClass ||
        !DamageEffect ||
        !TriggerEventData)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "FIREBALL validation failed: "
                "Character=%s ASC=%s ProjectileClass=%s "
                "DamageEffect=%s EventData=%s"),
            Character ? TEXT("valid") : TEXT("null"),
            SourceASC ? TEXT("valid") : TEXT("null"),
            ProjectileClass ? TEXT("valid") : TEXT("null"),
            DamageEffect ? TEXT("valid") : TEXT("null"),
            TriggerEventData ? TEXT("valid") : TEXT("null"));

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(
        TriggerEventData->TargetData,
        0))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("FIREBALL: Gameplay event has no cursor hit result"));

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const FHitResult CursorHit =
        UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(
            TriggerEventData->TargetData,
            0);

    const FVector TargetLocation = CursorHit.ImpactPoint;

    // Applies the configured Gameplay Ability cost and cooldown.
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("FIREBALL: CommitAbility failed"));

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector SpawnLocation =
        Character->GetActorLocation() +
        Character->GetActorForwardVector() * 100.0f;

    if (Character->GetMesh() &&
        Character->GetMesh()->DoesSocketExist(MuzzleSocketName))
    {
        SpawnLocation =
            Character->GetMesh()->GetSocketLocation(MuzzleSocketName);
    }

    FVector AdjustedTargetLocation = TargetLocation;

    FVector AimLocation = TargetLocation;

    // Guarantee horizontal movement from the actual spawn position.
    AimLocation.Z = SpawnLocation.Z;

    const FVector Direction =
        (AimLocation - SpawnLocation).GetSafeNormal2D();

    if (Direction.IsNearlyZero())
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

    Character->SetActorRotation(Direction.Rotation());

    Character->SetActorRotation(Direction.Rotation());

    if (CastMontage)
    {
        Character->PlayAnimMontage(CastMontage);
    }

    if (Character->HasAuthority())
    {
        FGameplayEffectContextHandle EffectContext =
            SourceASC->MakeEffectContext();

        EffectContext.AddSourceObject(Character);

        FGameplayEffectSpecHandle DamageSpec =
            SourceASC->MakeOutgoingSpec(
                DamageEffect,
                GetAbilityLevel(),
                EffectContext);

        if (DamageSpec.IsValid())
        {
            const FTransform SpawnTransform(
                Direction.Rotation(),
                SpawnLocation);

            AGenericProjectile* Projectile =
                GetWorld()->SpawnActorDeferred<AGenericProjectile>(
                    ProjectileClass,
                    SpawnTransform,
                    Character,
                    Character,
                    ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

            if (Projectile)
            {
                Projectile->FinishSpawning(SpawnTransform);

                Projectile->InitializeProjectile(
                    SourceASC,
                    DamageSpec,
                    Direction);
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}