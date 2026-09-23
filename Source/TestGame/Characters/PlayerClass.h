#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EPlayerClass : uint8
{
    None    UMETA(DisplayName = "None"),
    Warrior UMETA(DisplayName = "Warrior"),
    Mage    UMETA(DisplayName = "Mage"),
    Ranger  UMETA(DisplayName = "Ranger")
};