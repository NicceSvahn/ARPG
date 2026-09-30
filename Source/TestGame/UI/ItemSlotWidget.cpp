#include "ItemSlotWidget.h"

#include "ItemTooltipWidget.h"
#include "../Items/ItemRarity.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UItemSlotWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (SlotButton)
    {
        SlotButton->OnClicked.AddUniqueDynamic(
            this,
            &UItemSlotWidget::HandleClicked
        );
    }
}

void UItemSlotWidget::SetItem(
    int32 InIndex,
    const FInventoryItem& InItem)
{
    ItemIndex = InIndex;
    ItemId = InItem.InstanceId;

    UItemDefinition* Definition =
        InItem.IsValid()
        ? InItem.Definition.Get()
        : nullptr;

    if (ItemIcon)
    {
        ItemIcon->SetBrushFromTexture(
            Definition ? Definition->Icon.Get() : nullptr
        );

        ItemIcon->SetVisibility(
            Definition && Definition->Icon
            ? ESlateVisibility::HitTestInvisible
            : ESlateVisibility::Collapsed
        );
    }

    if (QuantityText)
    {
        // One item per slot for now.
        QuantityText->SetText(FText::GetEmpty());
        QuantityText->SetVisibility(
            ESlateVisibility::Collapsed
        );
    }

    if (EmptySlotText)
    {
        EmptySlotText->SetVisibility(
            Definition
            ? ESlateVisibility::Collapsed
            : ESlateVisibility::HitTestInvisible
        );
    }

    SetToolTipText(
        Definition
        ? FText::Format(
            NSLOCTEXT(
                "Inventory",
                "ItemTooltip",
                "{0}\n{1}"
            ),
            Definition->DisplayName,
            Definition->Description
        )
        : FText::GetEmpty()
    );

    UpdateRarityVisuals(InItem);
    UpdateItemTooltip(InItem);
}

void UItemSlotWidget::HandleClicked()
{
    if (ItemIndex != INDEX_NONE && ItemId.IsValid())
    {
        OnSlotPressed.Broadcast(
            ItemIndex,
            ItemId
        );
    }
}

void UItemSlotWidget::UpdateItemTooltip(
    const FInventoryItem& Item)
{
    // Remove any previous tooltip on the slot widget.
    SetToolTip(nullptr);
    SetToolTipText(FText::GetEmpty());

    if (!SlotButton)
    {
        return;
    }

    // Clear the button's tooltip when the item changes.
    SlotButton->SetToolTip(nullptr);
    SlotButton->SetToolTipText(FText::GetEmpty());

    if (!Item.IsValid() ||
        !IsValid(Item.Definition.Get()) ||
        !ItemTooltipClass)
    {
        return;
    }

    UItemTooltipWidget* Tooltip =
        CreateWidget<UItemTooltipWidget>(
            GetOwningPlayer(),
            ItemTooltipClass
        );

    if (!Tooltip)
    {
        return;
    }

    Tooltip->SetItem(Item);

    SlotButton->SetToolTip(Tooltip);
}

void UItemSlotWidget::UpdateRarityVisuals(
    const FInventoryItem& Item)
{
    if (!RarityBorder)
    {
        return;
    }

    // Neutral border for empty slots.
    FLinearColor BorderColor(
        0.08f,
        0.08f,
        0.08f,
        1.0f
    );

    const UItemDefinition* Definition =
        Item.Definition.Get();

    if (Item.IsValid() && IsValid(Definition))
    {
        BorderColor =
            ItemRarity::GetColor(Definition->Rarity);
    }

    RarityBorder->SetBrushColor(BorderColor);
}