#pragma once

#include "CoreMinimal.h"
#include "../GA_GenericAbility.h"
#include "GameplayEffectTypes.h"
#include "GA_ProjectileAbility.generated.h"

class AGenericCharacter;
class AGenericProjectile;
class UGameplayEffect;
class UTestGameAbilitySystemComponent;

USTRUCT()
struct FProjectileAbilityContext
{
    GENERATED_BODY()

    AGenericCharacter* Character = nullptr;
    UTestGameAbilitySystemComponent* ASC = nullptr;

    FVector TargetLocation = FVector::ZeroVector;
    FVector SpawnLocation = FVector::ZeroVector;
    FVector BaseDirection = FVector::ZeroVector;
};

UCLASS(Abstract)
class TESTGAME_API UGA_ProjectileAbility : public UGA_GenericAbility
{
    GENERATED_BODY()

public:
    UGA_ProjectileAbility();

protected:
    FGameplayEffectSpecHandle CreateProjectileDamageSpec(
        UTestGameAbilitySystemComponent* ASC,
        AGenericCharacter* Character,
        float Damage
    ) const;

    AGenericProjectile* SpawnProjectile(
        AGenericCharacter* Character,
        UTestGameAbilitySystemComponent* ASC,
        const FVector& SpawnLocation,
        const FVector& LaunchDirection,
        const FGameplayEffectSpecHandle& DamageSpec,
        const FGameplayEffectSpecHandle& AdditionalEffectSpec =
        FGameplayEffectSpecHandle()
    );

    virtual void SpawnProjectiles(
        const FProjectileAbilityContext& ProjectileContext
    );

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    TSubclassOf<AGenericProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    TSubclassOf<UGameplayEffect> DamageEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    FName MuzzleSocketName = TEXT("ProjectileSocket");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float SpawnForwardOffset = 30.0f;

    bool PrepareProjectileAbilityFromTargetData(
        const FGameplayAbilityTargetDataHandle& Data,
        FProjectileAbilityContext& OutContext
    );

    virtual void OnTargetDataReady(
        const FGameplayAbilityTargetDataHandle& Data
    ) override;
};