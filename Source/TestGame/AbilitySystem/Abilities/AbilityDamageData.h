#pragma once

#include "CoreMinimal.h"
#include "AbilityDamageData.generated.h"

UENUM(BlueprintType)
enum class EAbilityDamageType : uint8
{
    Physical UMETA(DisplayName = "Physical"),
    Magic UMETA(DisplayName = "Magic")
};

USTRUCT(BlueprintType)
struct FAbilityDamageData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float BaseDamage = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float WeaponDamageCoefficient = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float StrengthCoefficient = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float DexterityCoefficient = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float IntellectCoefficient = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float AttackPowerCoefficient = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float SpellPowerCoefficient = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    EAbilityDamageType DamageType = EAbilityDamageType::Physical;
};
