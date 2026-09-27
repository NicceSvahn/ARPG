#include "FloatingDamageWidget.h"

#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UFloatingDamageWidget::SetCombatText(
    const FCombatTextData& Data)
{
    if (!DamageText)
    {
        return;
    }

    DamageText->SetText(
        BuildDisplayText(Data)
    );

    DamageText->SetColorAndOpacity(
        FSlateColor(
            GetCombatTextColor(Data.Type)
        )
    );

    if (Data.Type == ECombatTextType::Critical)
    {
        DamageText->SetRenderScale(
            FVector2D(1.25f, 1.25f)
        );
    }
}

FText UFloatingDamageWidget::BuildDisplayText(
    const FCombatTextData& Data) const
{
    const int32 RoundedAmount =
        FMath::RoundToInt(Data.Amount);

    switch (Data.Type)
    {
    case ECombatTextType::Healing:
        return FText::FromString(
            FString::Printf(
                TEXT("+%d"),
                RoundedAmount
            )
        );

    case ECombatTextType::Block:
        return FText::FromString(TEXT("BLOCK"));

    case ECombatTextType::Absorb:
        return FText::FromString(
            FString::Printf(
                TEXT("ABSORB %d"),
                RoundedAmount
            )
        );

    case ECombatTextType::Miss:
        return FText::FromString(TEXT("MISS"));

    case ECombatTextType::Dodge:
        return FText::FromString(TEXT("DODGE"));

    case ECombatTextType::Resist:
        return FText::FromString(TEXT("RESIST"));

    case ECombatTextType::Immune:
        return FText::FromString(TEXT("IMMUNE"));

    case ECombatTextType::Critical:
        return FText::FromString(
            FString::Printf(
                TEXT("%d!"),
                RoundedAmount
            )
        );

    case ECombatTextType::Damage:
    default:
        return FText::AsNumber(RoundedAmount);
    }
}

FLinearColor UFloatingDamageWidget::GetCombatTextColor(
    const ECombatTextType Type) const
{
    switch (Type)
    {
    case ECombatTextType::Healing:
        return FLinearColor(0.1f, 1.0f, 0.1f);

    case ECombatTextType::Critical:
        return FLinearColor(1.0f, 0.7f, 0.1f);

    case ECombatTextType::Block:
    case ECombatTextType::Absorb:
        return FLinearColor(0.4f, 0.7f, 1.0f);

    case ECombatTextType::Miss:
    case ECombatTextType::Dodge:
    case ECombatTextType::Resist:
    case ECombatTextType::Immune:
        return FLinearColor(0.7f, 0.7f, 0.7f);

    case ECombatTextType::Damage:
    default:
        return FLinearColor::White;
    }
}

void UFloatingDamageWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (FloatingDamageAnim)
    {
        PlayAnimation(FloatingDamageAnim);
    }
}