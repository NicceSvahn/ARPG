#pragma once

#include "CoreMinimal.h"
#include "CombatTextData.generated.h"

UENUM(BlueprintType)
enum class ECombatTextType : uint8
{
    Damage,
    Critical,
    Healing,
    Block,
    Absorb,
    Miss,
    Dodge,
    Resist,
    Immune
};

USTRUCT(BlueprintType)
struct TESTGAME_API FCombatTextData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ECombatTextType Type = ECombatTextType::Damage;

    UPROPERTY(BlueprintReadWrite)
    float Amount = 0.0f;
};