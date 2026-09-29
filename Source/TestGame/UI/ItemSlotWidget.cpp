#include "ItemSlotWidget.h"

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