#pragma once

#include "CoreMinimal.h"
#include "ItemRarity.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Legendary UMETA(DisplayName = "Legendary"),
    Rare      UMETA(DisplayName = "Rare"),
    Magic     UMETA(DisplayName = "Magic"),
    Normal    UMETA(DisplayName = "Normal")
};

namespace ItemRarity
{
    inline FLinearColor GetColor(EItemRarity Rarity)
    {
        switch (Rarity)
        {
        case EItemRarity::Legendary:
            return FLinearColor(1.0f, 0.3f, 0.0f, 1.0f);

        case EItemRarity::Rare:
            return FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);

        case EItemRarity::Magic:
            return FLinearColor(0.1f, 0.35f, 1.0f, 1.0f);

        case EItemRarity::Normal:
        default:
            return FLinearColor::White;
        }
    }

    
    inline FText GetDisplayName(EItemRarity Rarity)
    {
        switch (Rarity)
        {
        case EItemRarity::Legendary:
            return NSLOCTEXT("ItemRarity", "Legendary", "Legendary");

        case EItemRarity::Rare:
            return NSLOCTEXT("ItemRarity", "Rare", "Rare");

        case EItemRarity::Magic:
            return NSLOCTEXT("ItemRarity", "Magic", "Magic");

        case EItemRarity::Normal:
        default:
            return NSLOCTEXT("ItemRarity", "Normal", "Normal");
        }
    }
}