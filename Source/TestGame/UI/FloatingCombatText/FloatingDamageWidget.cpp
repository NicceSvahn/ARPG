#include "FloatingDamageWidget.h"

#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UFloatingDamageWidget::SetDamageValue(
    float DamageAmount)
{
    if (!DamageText)
    {
        return;
    }

    DamageText->SetText(
        FText::AsNumber(
            FMath::RoundToInt(DamageAmount)
        )
    );
}

void UFloatingDamageWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (FloatingDamageAnim)
    {
        PlayAnimation(FloatingDamageAnim);
    }
}