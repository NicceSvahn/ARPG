#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ItemRarity.h"

#include "ItemGenerationProfile.generated.h"

USTRUCT(BlueprintType)
struct TESTGAME_API FItemAffixCountRule
{
    GENERATED_BODY()

    // Matches a GroupName on the item definition.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation")
    FName GroupName = NAME_None;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Generation",
        meta = (ClampMin = "0")
    )
    int32 MinCount = 0;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Generation",
        meta = (ClampMin = "0")
    )
    int32 MaxCount = 0;
};

UCLASS(BlueprintType)
class TESTGAME_API UItemGenerationProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation")
    EItemRarity Rarity = EItemRarity::Normal;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Generation",
        meta = (TitleProperty = "GroupName")
    )
    TArray<FItemAffixCountRule> AffixRules;
};
