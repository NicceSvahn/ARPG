#include "ExecCalc_Damage.h"

#include "../Attributes/PrimaryAttributeSet.h"
#include "../Attributes/OffensiveAttributeSet.h"
#include "../Attributes/DefensiveAttributeSet.h"
#include "../Attributes/HealthAttributeSet.h"

namespace
{
    struct FDamageStatics
    {
        DECLARE_ATTRIBUTE_CAPTUREDEF(Strength);
        DECLARE_ATTRIBUTE_CAPTUREDEF(Dexterity);
        DECLARE_ATTRIBUTE_CAPTUREDEF(Intellect);

        DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamage);
        DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
        DECLARE_ATTRIBUTE_CAPTUREDEF(SpellPower);
        DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamage);
        DECLARE_ATTRIBUTE_CAPTUREDEF(MagicDamage);
        DECLARE_ATTRIBUTE_CAPTUREDEF(CritChance);
        DECLARE_ATTRIBUTE_CAPTUREDEF(CritDamage);

        DECLARE_ATTRIBUTE_CAPTUREDEF(Armour);
        DECLARE_ATTRIBUTE_CAPTUREDEF(MagicResistance);

        FDamageStatics()
        {
            DEFINE_ATTRIBUTE_CAPTUREDEF(UPrimaryAttributeSet, Strength, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UPrimaryAttributeSet, Dexterity, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UPrimaryAttributeSet, Intellect, Source, false);

            DEFINE_ATTRIBUTE_CAPTUREDEF(UOffensiveAttributeSet, WeaponDamage, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UOffensiveAttributeSet, AttackPower, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UOffensiveAttributeSet, SpellPower, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UOffensiveAttributeSet, PhysicalDamage, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UOffensiveAttributeSet, MagicDamage, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UOffensiveAttributeSet, CritChance, Source, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UOffensiveAttributeSet, CritDamage, Source, false);

            DEFINE_ATTRIBUTE_CAPTUREDEF(UDefensiveAttributeSet, Armour, Target, false);
            DEFINE_ATTRIBUTE_CAPTUREDEF(UDefensiveAttributeSet, MagicResistance, Target, false);
        }
    };

    const FDamageStatics& DamageStatics()
    {
        static FDamageStatics Statics;
        return Statics;
    }

    float CaptureMagnitude(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        const FGameplayEffectAttributeCaptureDefinition& Definition,
        const FAggregatorEvaluateParameters& EvaluationParameters)
    {
        float Value = 0.0f;
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
            Definition,
            EvaluationParameters,
            Value
        );
        return Value;
    }
}

UExecCalc_Damage::UExecCalc_Damage()
{
    RelevantAttributesToCapture.Add(DamageStatics().StrengthDef);
    RelevantAttributesToCapture.Add(DamageStatics().DexterityDef);
    RelevantAttributesToCapture.Add(DamageStatics().IntellectDef);

    RelevantAttributesToCapture.Add(DamageStatics().WeaponDamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().SpellPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().PhysicalDamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().MagicDamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().CritChanceDef);
    RelevantAttributesToCapture.Add(DamageStatics().CritDamageDef);

    RelevantAttributesToCapture.Add(DamageStatics().ArmourDef);
    RelevantAttributesToCapture.Add(DamageStatics().MagicResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    static const FGameplayTag DataDamageBaseTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.Base"));

    static const FGameplayTag DataDamageWeaponCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.WeaponCoefficient"));

    static const FGameplayTag DataDamageStrengthCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.StrengthCoefficient"));

    static const FGameplayTag DataDamageDexterityCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.DexterityCoefficient"));

    static const FGameplayTag DataDamageIntellectCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.IntellectCoefficient"));

    static const FGameplayTag DataDamageAttackPowerCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.AttackPowerCoefficient"));

    static const FGameplayTag DataDamageSpellPowerCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.SpellPowerCoefficient"));

    static const FGameplayTag DataDamageIsMagicTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.IsMagic"));


    const float BaseDamage = Spec.GetSetByCallerMagnitude(
        DataDamageBaseTag,
        false,
        0.0f
    );

    const float WeaponCoefficient = Spec.GetSetByCallerMagnitude(
        DataDamageWeaponCoefficientTag,
        false,
        0.0f
    );

    const float StrengthCoefficient = Spec.GetSetByCallerMagnitude(
        DataDamageStrengthCoefficientTag,
        false,
        0.0f
    );

    const float DexterityCoefficient = Spec.GetSetByCallerMagnitude(
        DataDamageDexterityCoefficientTag,
        false,
        0.0f
    );

    const float IntellectCoefficient = Spec.GetSetByCallerMagnitude(
        DataDamageIntellectCoefficientTag,
        false,
        0.0f
    );

    const float AttackPowerCoefficient = Spec.GetSetByCallerMagnitude(
        DataDamageAttackPowerCoefficientTag,
        false,
        0.0f
    );

    const float SpellPowerCoefficient = Spec.GetSetByCallerMagnitude(
        DataDamageSpellPowerCoefficientTag,
        false,
        0.0f
    );

    const bool bIsMagic = Spec.GetSetByCallerMagnitude(
        DataDamageIsMagicTag,
        false,
        0.0f
    ) > 0.5f;

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    const float Strength = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().StrengthDef, EvaluationParameters),
        0.0f
    );

    const float Dexterity = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().DexterityDef, EvaluationParameters),
        0.0f
    );

    const float Intellect = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().IntellectDef, EvaluationParameters),
        0.0f
    );

    const float WeaponDamage = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().WeaponDamageDef, EvaluationParameters),
        0.0f
    );

    const float AttackPower = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().AttackPowerDef, EvaluationParameters),
        0.0f
    );

    const float SpellPower = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().SpellPowerDef, EvaluationParameters),
        0.0f
    );

    const float PhysicalDamageBonus = CaptureMagnitude(
        ExecutionParams,
        DamageStatics().PhysicalDamageDef,
        EvaluationParameters
    );

    const float MagicDamageBonus = CaptureMagnitude(
        ExecutionParams,
        DamageStatics().MagicDamageDef,
        EvaluationParameters
    );

    const float CritChance = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().CritChanceDef, EvaluationParameters),
        0.0f
    );

    const float CritDamage = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().CritDamageDef, EvaluationParameters),
        0.0f
    );

    const float Armour = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().ArmourDef, EvaluationParameters),
        0.0f
    );

    const float MagicResistance = FMath::Max(
        CaptureMagnitude(ExecutionParams, DamageStatics().MagicResistanceDef, EvaluationParameters),
        0.0f
    );

    const float RawDamage = FMath::Max(
        BaseDamage
        + WeaponDamage * WeaponCoefficient
        + Strength * StrengthCoefficient
        + Dexterity * DexterityCoefficient
        + Intellect * IntellectCoefficient
        + AttackPower * AttackPowerCoefficient
        + SpellPower * SpellPowerCoefficient,
        0.0f
    );

    const float DamageBonusPercent = bIsMagic
        ? MagicDamageBonus
        : PhysicalDamageBonus;

    float FinalDamage = RawDamage * FMath::Max(
        1.0f + DamageBonusPercent / 100.0f,
        0.0f
    );

    const float CritChanceNormalized = FMath::Clamp(
        CritChance / 100.0f,
        0.0f,
        1.0f
    );

    const bool bCriticalHit = FMath::FRand() < CritChanceNormalized;

    if (bCriticalHit)
    {
        FinalDamage *= 1.0f + CritDamage / 100.0f;
    }

    // Initial tuning constant. 400 armour/resistance = 50% mitigation.
    // Move this to a CurveTable/DataAsset later when you begin level scaling.
    constexpr float MitigationConstant = 400.0f;

    const float MitigationStat = bIsMagic
        ? MagicResistance
        : Armour;

    const float MitigationPercent = MitigationStat > 0.0f
        ? MitigationStat / (MitigationStat + MitigationConstant)
        : 0.0f;

    const float DamageAfterMitigation = FMath::Max(
        FinalDamage * (1.0f - MitigationPercent),
        0.0f
    );

    if (DamageAfterMitigation <= 0.0f)
    {
        return;
    }

    /*
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("DamageCalc Type=%s Base=%.2f Weapon=%.2f STR=%.2f DEX=%.2f INT=%.2f AP=%.2f SP=%.2f Raw=%.2f Bonus=%.2f%% Crit=%s Mitigation=%.2f%% Result=%.2f"),
        bIsMagic ? TEXT("Magic") : TEXT("Physical"),
        BaseDamage,
        WeaponDamage,
        Strength,
        Dexterity,
        Intellect,
        AttackPower,
        SpellPower,
        RawDamage,
        DamageBonusPercent,
        bCriticalHit ? TEXT("YES") : TEXT("NO"),
        MitigationPercent * 100.0f,
        DamageAfterMitigation
    );
    */

    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            UHealthAttributeSet::GetIncomingDamageAttribute(),
            EGameplayModOp::Additive,
            DamageAfterMitigation
        )
    );
}
