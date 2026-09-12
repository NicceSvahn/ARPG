#include "GA_ProjectileAbility.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "../../../Characters/GenericCharacter.h"
#include "../../../Characters/EnemyCharacter.h"
#include "../../TestGameAbilitySystemComponent.h"
#include "../../AbilityInputContext.h"
#include "../GenericProjectile.h"

UGA_ProjectileAbility::UGA_ProjectileAbility()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGA_ProjectileAbility::PrepareProjectileAbility(
    FProjectileAbilityContext& OutContext)
{
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
        return false;
    }

    const FAbilityInputContext& InputContext =
        ASC->GetAbilityInputContext();

    FVector TargetLocation = FVector::ZeroVector;

    const bool bCasterIsEnemy =
        Character->IsA<AEnemyCharacter>();

    if (IsValid(InputContext.TargetActor) && bCasterIsEnemy)
    {
        // AI and actor-targeted abilities.
        TargetLocation =
            InputContext.TargetActor->GetActorLocation();
    }
    else if (!InputContext.HitResult.bBlockingHit)
    {
        return false;
    }
    else
    {
        // Explicit location targeting.
        TargetLocation =
            InputContext.HitLocation;
    }

    if (TargetLocation.IsNearlyZero())
    {
        return false;
    }

    FVector CharacterAimDirection =
        TargetLocation -
        Character->GetActorLocation();

    CharacterAimDirection.Z = 0.0f;

    if (CharacterAimDirection.IsNearlyZero())
    {
        return false;
    }

    Character->SetActorRotation(
        CharacterAimDirection.Rotation()
    );

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

    SpawnLocation +=
        Character->GetActorForwardVector() *
        SpawnForwardOffset;

    const FVector BaseDirection =
        (TargetLocation - SpawnLocation)
        .GetSafeNormal2D();

    if (BaseDirection.IsNearlyZero())
    {
        return false;
    }

    OutContext.Character = Character;
    OutContext.ASC = ASC;
    OutContext.TargetLocation = TargetLocation;
    OutContext.SpawnLocation = SpawnLocation;
    OutContext.BaseDirection = BaseDirection;

    return true;
}

FGameplayEffectSpecHandle
UGA_ProjectileAbility::CreateProjectileDamageSpec(
    UTestGameAbilitySystemComponent* ASC,
    AGenericCharacter* Character,
    float Damage
) const
{
    if (!ASC ||
        !Character ||
        !DamageEffect)
    {
        return FGameplayEffectSpecHandle();
    }

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
        return FGameplayEffectSpecHandle();
    }

    const FGameplayTag DamageTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Data.Damage")
        );

    DamageSpec.Data->SetSetByCallerMagnitude(
        DamageTag,
        Damage
    );

    return DamageSpec;
}

AGenericProjectile*
UGA_ProjectileAbility::SpawnProjectile(
    AGenericCharacter* Character,
    UTestGameAbilitySystemComponent* ASC,
    const FVector& SpawnLocation,
    const FVector& LaunchDirection,
    const FGameplayEffectSpecHandle& DamageSpec
)
{
    if (!Character ||
        !ASC ||
        !ProjectileClass ||
        !DamageSpec.IsValid())
    {
        return nullptr;
    }

    if (!Character->HasAuthority())
    {
        return nullptr;
    }

    const FVector SafeDirection =
        LaunchDirection.GetSafeNormal2D();

    if (SafeDirection.IsNearlyZero())
    {
        return nullptr;
    }

    const FTransform SpawnTransform(
        SafeDirection.Rotation(),
        SpawnLocation
    );

    AGenericProjectile* Projectile =
        GetWorld()->SpawnActorDeferred<AGenericProjectile>(
            ProjectileClass,
            SpawnTransform,
            Character,
            Character,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

    if (!Projectile)
    {
        return nullptr;
    }

    Projectile->InitializeProjectile(
        ASC,
        DamageSpec,
        SafeDirection
    );

    Projectile->FinishSpawning(
        SpawnTransform
    );

    return Projectile;
}