#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

class UTexture2D;
class UGameplayEffect;


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

    bool IsValid() const
    {
        return InstanceId.IsValid() && Definition != nullptr;
    }
};