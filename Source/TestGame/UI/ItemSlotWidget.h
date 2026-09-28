#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Items/ItemDefinition.h"
#include "ItemSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_TwoParams(
    FOnItemSlotPressed,
    int32,
    FGuid
);

UCLASS()
class TESTGAME_API UItemSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Item Slot"
    )
    bool bEquipmentSlot = false;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Item Slot"
    )
    EEquipmentSlot EquipmentSlot = EEquipmentSlot::Head;

    FOnItemSlotPressed OnSlotPressed;

    void SetItem(
        int32 InIndex,
        const FInventoryItem& InItem
    );

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleClicked();

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> SlotButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> QuantityText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> EmptySlotText;

    int32 ItemIndex = INDEX_NONE;
    FGuid ItemId;
};