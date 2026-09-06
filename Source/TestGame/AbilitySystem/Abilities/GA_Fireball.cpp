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

    AGenericCharacter* Character = GetGenericCharacter();

    UAbilitySystemComponent* SourceASC =
        GetAbilitySystemComponentFromActorInfo();

    if (!Character ||
        !SourceASC ||
        !ProjectileClass ||
        !DamageEffect ||
        !TriggerEventData)
    {
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


    const FVector Direction =
        (TargetLocation - SpawnLocation).GetSafeNormal();

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
            // Add this only if the damage Gameplay Effect uses this tag.
            const FGameplayTag DamageTag =
                FGameplayTag::RequestGameplayTag(
                    TEXT("Data.Damage"));

            DamageSpec.Data->SetSetByCallerMagnitude(
                DamageTag,
                FireballDamage);

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
                Projectile->InitializeProjectile(
                    SourceASC,
                    DamageSpec,
                    nullptr);

                Projectile->FinishSpawning(SpawnTransform);
            }
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}