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
    
    bRequiresTargetData = true;

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

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[PROJECTILE CONTEXT] Character=%s | Authority=%s | "
            "BlockingHit=%s | Target=%s | HitLocation=%s"
        ),
        *GetNameSafe(Character),
        Character->HasAuthority()
        ? TEXT("TRUE")
        : TEXT("FALSE"),
        InputContext.HitResult.bBlockingHit
        ? TEXT("TRUE")
        : TEXT("FALSE"),
        *GetNameSafe(InputContext.TargetActor),
        *InputContext.HitLocation.ToString()
    );

    return PrepareProjectileAbilityFromHitResult(
        InputContext.HitResult,
        OutContext
    );
}

bool UGA_ProjectileAbility::PrepareProjectileAbilityFromHitResult(
    const FHitResult& HitResult,
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
        !DamageEffect ||
        !HitResult.bBlockingHit)
    {
        return false;
    }

    FVector TargetLocation =
        HitResult.ImpactPoint;

    if (IsValid(HitResult.GetActor()) &&
        Character->IsA<AEnemyCharacter>())
    {
        TargetLocation =
            HitResult.GetActor()->GetActorLocation();
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

void UGA_ProjectileAbility::SpawnProjectiles(
    const FProjectileAbilityContext& ProjectileContext)
{
}

void UGA_ProjectileAbility::OnTargetDataReady(
    const FGameplayAbilityTargetDataHandle& Data)
{
    if (Data.Num() <= 0)
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    const FHitResult* HitResult =
        Data.Get(0)->GetHitResult();

    if (!HitResult)
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[PROJECTILE TARGET DATA] "
            "HitLocation=%s | Target=%s"
        ),
        *HitResult->ImpactPoint.ToString(),
        *GetNameSafe(HitResult->GetActor())
    );

    FProjectileAbilityContext ProjectileContext;

    if (!PrepareProjectileAbilityFromHitResult(
        *HitResult,
        ProjectileContext))
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    if (!CommitAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo()))
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    if (ProjectileContext.Character->HasAuthority())
    {
        SpawnProjectiles(
            ProjectileContext
        );
    }

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true,
        false
    );
}