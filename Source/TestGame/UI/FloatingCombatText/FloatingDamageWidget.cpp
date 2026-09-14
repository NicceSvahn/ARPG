#include "FloatingDamageWidget.h"

#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UFloatingDamageWidget::SetDamageValue(
    float DamageAmount)
{
    SetCombatTextValue(
        DamageAmount,
        FLinearColor::White
    );
}

void UFloatingDamageWidget::SetCombatTextValue(float Amount, const FLinearColor& Color)
{
    if (!DamageText)
    {
        return;
    }

    const int32 RoundedAmount = FMath::RoundToInt(Amount);
    const FString DisplayString = FString::FromInt(RoundedAmount);

    DamageText->SetText(FText::FromString(DisplayString));
    DamageText->SetColorAndOpacity(FSlateColor(Color));
}

void UFloatingDamageWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (FloatingDamageAnim)
    {
        PlayAnimation(FloatingDamageAnim);
    }
}