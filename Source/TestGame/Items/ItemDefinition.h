#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "ItemGenerationProfile.h"
#include "ItemRarity.h"

#include "ItemDefinition.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UStaticMesh;
class UTexture2D;


/**
 * Describes the broad animation category of an equipped weapon.
 *
 * Nothing uses this to select ability animations yet.
 * It is stored now so the animation system can optionally use it later.
 */
UENUM(BlueprintType)
enum class EWeaponAnimationType : uint8
{
    None UMETA(DisplayName = "None"),

    OneHandedSword UMETA(DisplayName = "One-Handed Sword"),
    OneHandedAxe UMETA(DisplayName = "One-Handed Axe"),
    OneHandedMace UMETA(DisplayName = "One-Handed Mace"),

    TwoHandedSword UMETA(DisplayName = "Two-Handed Sword"),
    TwoHandedAxe UMETA(DisplayName = "Two-Handed Axe"),
    Polearm UMETA(DisplayName = "Polearm"),

    Bow UMETA(DisplayName = "Bow"),
    Staff UMETA(DisplayName = "Staff"),
    Shield UMETA(DisplayName = "Shield")
};


/**
 * Presentation data used while an item is equipped.
 *
 * This does not affect gameplay, stats, item generation or abilities.
 */
USTRUCT(BlueprintType)
struct TESTGAME_API FEquipmentVisualData
{
    GENERATED_BODY()

    /**
     * Mesh displayed while the item is equipped.
     *
     * Normal swords, axes, shields, etc. can generally use a Static Mesh.
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment|Visual"
    )
    TObjectPtr<UStaticMesh> StaticMesh = nullptr;

    /**
     * Socket used when this item is equipped in MainHand.
     *
     * Example:
     * weapon_r
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment|Visual"
    )
    FName MainHandSocket = NAME_None;

    /**
     * Socket used when this item is equipped in OffHand.
     *
     * Example:
     * weapon_l
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment|Visual"
    )
    FName OffHandSocket = NAME_None;

    /**
     * Per-item correction applied after attaching to the socket.
     *
     * Useful if different weapon assets have different pivots/orientation.
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment|Visual"
    )
    FTransform RelativeTransform = FTransform::Identity;

    /**
     * Broad animation category for future animation logic.
     *
     * Abilities still use their own montages unless explicitly
     * changed later.
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment|Animation"
    )
    EWeaponAnimationType WeaponAnimationType =
        EWeaponAnimationType::None;
};


// Defines the possible values for one stat.
USTRUCT(BlueprintType)
struct TESTGAME_API FItemStatRange
{
    GENERATED_BODY()

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Item"
    )
    FGameplayTag StatTag;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Item"
    )
    FText DisplayName;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Item"
    )
    int32 MinValue = 1;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Item"
    )
    int32 MaxValue = 10;
};


USTRUCT(BlueprintType)
struct TESTGAME_API FItemStatGroup
{
    GENERATED_BODY()

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Stats"
    )
    FName GroupName = NAME_None;

    // Every entry in this group is rolled.
    // Rarity rules do not affect these stats.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Stats"
    )
    bool bGuaranteed = false;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Stats",
        meta = (TitleProperty = "DisplayName")
    )
    TArray<FItemStatRange> StatPool;
};


USTRUCT(BlueprintType)
struct TESTGAME_API FItemGenerationChoice
{
    GENERATED_BODY()

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Generation"
    )
    TObjectPtr<UItemGenerationProfile> Profile = nullptr;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Generation",
        meta = (
            ClampMin = "0",
            ClampMax = "100"
            )
    )
    int32 ChancePercent = 0;
};


// Stores the actual value rolled for an individual item.
USTRUCT(BlueprintType)
struct TESTGAME_API FItemRolledStat
{
    GENERATED_BODY()

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    FGameplayTag StatTag;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    FText DisplayName;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    float Value = 0.0f;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    bool bGuaranteed = false;
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

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    FGuid InstanceId;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    TObjectPtr<UItemDefinition> Definition = nullptr;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    TArray<FItemRolledStat> RolledStats;

    UPROPERTY(
        BlueprintReadOnly,
        Category = "Item"
    )
    EItemRarity Rarity =
        EItemRarity::Normal;

    bool IsValid() const
    {
        return
            InstanceId.IsValid() &&
            Definition != nullptr;
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

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item|Stats"
    )
    TArray<FItemStatRange> StatRanges;

    /**
     * Optional ability granted and activated while equipped.
     *
     * Existing PR #235 functionality.
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment"
    )
    TSubclassOf<UGameplayAbility> EquippedAbility;

    // Tooltip
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item|Display"
    )
    FText ItemTypeText;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item"
    )
    EItemRarity Rarity =
        EItemRarity::Normal;

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

    /**
     * Presentation-only information for this item.
     */
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Equipment|Visual"
    )
    FEquipmentVisualData EquipmentVisual;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item|Stats",
        meta = (TitleProperty = "GroupName")
    )
    TArray<FItemStatGroup> StatGroups;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Item|Generation"
    )
    TArray<FItemGenerationChoice> GenerationChoices;
};