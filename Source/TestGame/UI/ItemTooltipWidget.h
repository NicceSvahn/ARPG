#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Items/ItemDefinition.h"
#include "ItemTooltipWidget.generated.h"

class UTextBlock;

UCLASS()
class TESTGAME_API UItemTooltipWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetItem(const FInventoryItem& Item);

protected:

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ItemNameText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ItemTypeText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> RarityText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> EquipmentSlotText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DescriptionText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> StatsText;
};