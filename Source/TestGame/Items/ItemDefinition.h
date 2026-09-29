#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "ItemDefinition.generated.h"

class UTexture2D;
class UGameplayEffect;

// Defines the possible values for one stat.
USTRUCT(BlueprintType)
struct TESTGAME_API FItemStatRange
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FGameplayTag StatTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 MinValue = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 MaxValue = 10;
};

USTRUCT(BlueprintType)
struct TESTGAME_API FItemStatGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    FName GroupName = NAME_None;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Stats",
        meta = (ClampMin = "0")
    )
    int32 StatCount = 1;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Stats",
        meta = (TitleProperty = "DisplayName")
    )
    TArray<FItemStatRange> StatPool;
};

// Stores the actual value rolled for an individual item.
USTRUCT(BlueprintType)
struct TESTGAME_API FItemRolledStat
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Item")
    FGameplayTag StatTag;

    UPROPERTY(BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Item")
    float Value = 0.0f;
};

// The equipment positions available on a character.
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    Head,
    Shoulders,
    Amulet,
    Chest,
    Hands,
    Wrists,
    Belt,
    LeftRing,
    RightRing,
    Legs,
    Feet,
    MainHand,
    OffHand,

    // Used by C++ to determine the number of equipment slots.
    // This is not an actual equipment position.
    Count UMETA(Hidden)
};

// One particular copy of an item in an inventory.
USTRUCT(BlueprintType)
struct TESTGAME_API FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Item")
    FGuid InstanceId;

    UPROPERTY(BlueprintReadOnly, Category = "Item")
    TObjectPtr<UItemDefinition> Definition = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Item")
    TArray<FItemRolledStat> RolledStats;

    bool IsValid() const
    {
        return InstanceId.IsValid() && Definition != nullptr;
    }
};


// Shared information describing one type of item.
UCLASS(BlueprintType)
class TESTGAME_API UItemDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item"
    )
    FText DisplayName;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item",
        meta = (MultiLine = true)
    )
    FText Description;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item"
    )
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stats")
    TArray<FItemStatRange> StatRanges;

    //Tooltip
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
    FText ItemTypeText;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Display")
    FText RarityText;

    // An empty array means this item cannot be equipped.
    // A ring can support both LeftRing and RightRing.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment"
    )
    TArray<EEquipmentSlot> AllowedSlots;

    // Optional Gameplay Effect applied while equipped.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment"
    )
    TSubclassOf<UGameplayEffect> EquippedEffect;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item|Stats",
        meta = (TitleProperty = "GroupName")
    )
        TArray<FItemStatGroup> StatGroups;
};