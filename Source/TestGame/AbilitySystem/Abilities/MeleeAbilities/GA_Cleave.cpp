#include "GA_Cleave.h"

#include "../AoEAbilities/GA_GenericAoE.h"
#include "../../../Characters/EnemyCharacter.h"
#include "../../../Characters/GenericCharacter.h"

UGA_Cleave::UGA_Cleave()
{
    AffectedCharacterClass =
        AEnemyCharacter::StaticClass();

    DamageData.BaseDamage =
        20.0f;

    DamageData.WeaponDamageCoefficient =
        1.0f;

    DamageData.StrengthCoefficient =
        0.5f;

    DamageData.DexterityCoefficient =
        0.0f;

    DamageData.IntellectCoefficient =
        0.0f;

    DamageData.AttackPowerCoefficient =
        0.0f;

    DamageData.SpellPowerCoefficient =
        0.0f;

    DamageData.DamageType =
        EAbilityDamageType::Physical;

    ResourceGain =
        10.0f;
}

UAnimMontage* UGA_Cleave::GetAttackMontageForActivation()
{
    // If both alternating montages are configured, alternate between them
    // on every Cleave activation.
    if (LeftCleaveMontage && RightCleaveMontage)
    {
        UAnimMontage* MontageToPlay =
            bUseLeftMontageNext
            ? LeftCleaveMontage.Get()
            : RightCleaveMontage.Get();

        bUseLeftMontageNext =
            !bUseLeftMontageNext;

        return MontageToPlay;
    }

    // Allow development with only one of the two montages configured.
    if (LeftCleaveMontage)
    {
        return LeftCleaveMontage.Get();
    }

    if (RightCleaveMontage)
    {
        return RightCleaveMontage.Get();
    }

    // Backwards-compatible fallback to GenericMelee's AttackMontage.
    return Super::GetAttackMontageForActivation();
}

void UGA_Cleave::OnMeleeHit(
    AActor* TargetActor)
{
    AGenericCharacter* SourceCharacter =
        GetGenericCharacter();

    if (!SourceCharacter)
    {
        return;
    }

    ApplyCleaveDamage(
        SourceCharacter
    );
}

void UGA_Cleave::ApplyCleaveDamage(
    AGenericCharacter* SourceCharacter)
{
    if (
        !SourceCharacter ||
        !SourceCharacter->HasAuthority()
        )
    {
        return;
    }

    const TArray<AGenericCharacter*> Targets =
        UGA_GenericAoE::FindCharactersInArc(
            this,
            SourceCharacter->GetActorLocation(),
            SourceCharacter->GetActorForwardVector(),
            CleaveRange,
            CleaveArcDegrees,
            SourceCharacter,
            AffectedCharacterClass
        );

    bool bHitAnyTarget =
        false;

    for (AGenericCharacter* Target : Targets)
    {
        if (!IsValid(Target))
        {
            continue;
        }

        const bool bAppliedDamage =
            ApplyDamageToTarget(
                Target,
                DamageEffect,
                DamageData
            );

        if (bAppliedDamage)
        {
            bHitAnyTarget =
                true;
        }
    }

    if (bHitAnyTarget)
    {
        GrantResource(
            ResourceGain
        );
    }
}
