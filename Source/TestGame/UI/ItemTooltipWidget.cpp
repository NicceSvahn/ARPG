#include "ItemTooltipWidget.h"

#include "../Items/ItemRarity.h"

#include "Styling/SlateColor.h"
#include "Components/TextBlock.h"


namespace
{
    void SetOptionalText(
        UTextBlock* TextBlock,
        const FText& Text)
    {
        if (!TextBlock)
        {
            return;
        }

        TextBlock->SetText(Text);

        TextBlock->SetVisibility(
            Text.IsEmpty()
            ? ESlateVisibility::Collapsed
            : ESlateVisibility::HitTestInvisible
        );
    }
}

void UItemTooltipWidget::SetItem(
    const FInventoryItem& Item)
{
    const UItemDefinition* Definition =
        Item.Definition.Get();

    if (!Item.IsValid() || !IsValid(Definition))
    {
        SetOptionalText(ItemNameText, FText::GetEmpty());
        SetOptionalText(ItemTypeText, FText::GetEmpty());
        SetOptionalText(RarityText, FText::GetEmpty());
        SetOptionalText(EquipmentSlotText, FText::GetEmpty());
        SetOptionalText(DescriptionText, FText::GetEmpty());
        SetOptionalText(StatsText, FText::GetEmpty());

        return;
    }

    SetOptionalText(
        ItemNameText,
        Definition->DisplayName
    );

    SetOptionalText(
        ItemTypeText,
        Definition->ItemTypeText
    );

    SetOptionalText(
        RarityText,
        ItemRarity::GetDisplayName(Definition->Rarity)
    );

    if (RarityText)
    {
        RarityText->SetColorAndOpacity(
            FSlateColor(
                ItemRarity::GetColor(Definition->Rarity)
            )
        );
    }

    SetOptionalText(
        DescriptionText,
        Definition->Description
    );

    // Display the equipment slots this item supports.
    FText SlotNames;

    const UEnum* SlotEnum =
        StaticEnum<EEquipmentSlot>();

    if (SlotEnum)
    {
        for (const EEquipmentSlot EquipmentSlot :
        Definition->AllowedSlots)
        {
            if (EquipmentSlot == EEquipmentSlot::Count)
            {
                continue;
            }

            const FText SlotName =
                SlotEnum->GetDisplayNameTextByValue(
                    static_cast<int64>(EquipmentSlot)
                );

            if (SlotNames.IsEmpty())
            {
                SlotNames = SlotName;
            }
            else
            {
                SlotNames = FText::Format(
                    NSLOCTEXT(
                        "ItemTooltip",
                        "SlotList",
                        "{0}, {1}"
                    ),
                    SlotNames,
                    SlotName
                );
            }
        }
    }

    FText SlotLabel;

    if (!SlotNames.IsEmpty())
    {
        SlotLabel = FText::Format(
            NSLOCTEXT(
                "ItemTooltip",
                "EquipmentSlotLabel",
                "Equipment slot: {0}"
            ),
            SlotNames
        );
    }

    SetOptionalText(EquipmentSlotText, SlotLabel);

    // Read the stored results, never roll new values here.
    FText StatLines;

    for (const FItemRolledStat& Stat : Item.RolledStats)
    {
        const FText StatName =
            Stat.DisplayName.IsEmpty()
            ? FText::FromString(Stat.StatTag.ToString())
            : Stat.DisplayName;

        const FText StatLine = FText::Format(
            NSLOCTEXT(
                "ItemTooltip",
                "StatLine",
                "{0}: {1}"
            ),
            StatName,
            FText::AsNumber(Stat.Value)
        );

        if (StatLines.IsEmpty())
        {
            StatLines = StatLine;
        }
        else
        {
            StatLines = FText::Format(
                NSLOCTEXT(
                    "ItemTooltip",
                    "AppendStatLine",
                    "{0}\n{1}"
                ),
                StatLines,
                StatLine
            );
        }
    }

    SetOptionalText(StatsText, StatLines);
}
