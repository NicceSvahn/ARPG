#include "BuffIconWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBuffIconWidget::InitializeBuff(const FText& BuffName, UTexture2D* Icon)
{
    if (BuffNameText)
    {
        BuffNameText->SetText(BuffName);
    }

    if (BuffIcon && Icon)
    {
        BuffIcon->SetBrushFromTexture(Icon);
    }
}

void UBuffIconWidget::SetRemainingSeconds(float RemainingTime)
{
    if (!DurationText)
    {
        return;
    }

    if (RemainingTime < 0.0f)
    {
        DurationText->SetText(FText::GetEmpty());
        return;
    }

    const int32 Seconds = FMath::Max(0, FMath::CeilToInt(RemainingTime));
    DurationText->SetText(FText::AsNumber(Seconds));
}
