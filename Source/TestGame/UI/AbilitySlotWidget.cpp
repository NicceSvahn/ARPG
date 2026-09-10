#include "AbilitySlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../AbilitySystem/Abilities/GA_GenericAbility.h"

void UAbilitySlotWidget::InitializeSlot(
    UTestGameAbilitySystemComponent* InASC,
    const FGameplayTag& InInputTag,
    const FText& InInputLabel)
{
    AbilitySystemComponent = InASC;
    InputTag = InInputTag;

    if (InputText)
    {
        InputText->SetText(InInputLabel);
    }

    RefreshAbility();
}

void UAbilitySlotWidget::RefreshAbility()
{
    if (!Icon || !AbilitySystemComponent)
    {
        return;
    }

    UGameplayAbility* Ability =
        AbilitySystemComponent->GetAbilityForInputTag(
            InputTag);

    UGA_GenericAbility* GenericAbility =
        Cast<UGA_GenericAbility>(Ability);

    if (!GenericAbility)
    {
        Icon->SetBrushFromTexture(nullptr);
        Icon->SetVisibility(
            ESlateVisibility::Hidden);

        return;
    }

    UTexture2D* AbilityIcon =
        GenericAbility->GetAbilityIcon();

    if (!AbilityIcon)
    {
        Icon->SetBrushFromTexture(nullptr);
        Icon->SetVisibility(
            ESlateVisibility::Hidden);

        return;
    }

    Icon->SetBrushFromTexture(AbilityIcon);

    Icon->SetVisibility(
        ESlateVisibility::HitTestInvisible);
}

void UAbilitySlotWidget::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime)
{
    Super::NativeTick(
        MyGeometry,
        InDeltaTime
    );

    RefreshCooldown();
}

void UAbilitySlotWidget::RefreshCooldown()
{
    if (!AbilitySystemComponent ||
        !CooldownOverlay ||
        !CooldownText)
    {
        return;
    }

    UGameplayAbility* Ability =
        AbilitySystemComponent
        ->GetAbilityForInputTag(InputTag);

    const UGA_GenericAbility* GenericAbility =
        Cast<UGA_GenericAbility>(Ability);

    if (!GenericAbility)
    {
        CooldownOverlay->SetVisibility(
            ESlateVisibility::Collapsed
        );

        CooldownText->SetVisibility(
            ESlateVisibility::Collapsed
        );

        return;
    }

    const float RemainingTime =
        AbilitySystemComponent
        ->GetRemainingCooldown(
            GenericAbility->GetCooldownTag()
        );

    if (RemainingTime <= 0.0f)
    {
        CooldownOverlay->SetVisibility(
            ESlateVisibility::Collapsed
        );

        CooldownText->SetVisibility(
            ESlateVisibility::Collapsed
        );

        return;
    }

    CooldownOverlay->SetVisibility(
        ESlateVisibility::HitTestInvisible
    );

    CooldownText->SetVisibility(
        ESlateVisibility::HitTestInvisible
    );

    CooldownText->SetText(
        FText::AsNumber(
            FMath::CeilToInt(RemainingTime)
        )
    );
}