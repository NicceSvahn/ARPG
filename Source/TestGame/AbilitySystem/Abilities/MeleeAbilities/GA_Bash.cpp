#include "GA_Bash.h"

UGA_Bash::UGA_Bash()
{
    DamageData.BaseDamage = 20.0f;
    DamageData.WeaponDamageCoefficient = 1.0f;
    DamageData.StrengthCoefficient = 0.5f;
    DamageData.DexterityCoefficient = 0.0f;
    DamageData.IntellectCoefficient = 0.0f;
    DamageData.AttackPowerCoefficient = 0.0f;
    DamageData.SpellPowerCoefficient = 0.0f;
    DamageData.DamageType = EAbilityDamageType::Physical;
}
