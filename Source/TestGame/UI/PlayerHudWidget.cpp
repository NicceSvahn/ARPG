#include "PlayerHudWidget.h"

#include "AbilitySlotWidget.h"
#include "HealthBarWidget.h"
#include "ResourceBarWidget.h"
#include "Buffs/BuffBarWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

#include "../AbilitySystem/TestGameAbilitySystemComponent.h"

void UPlayerHudWidget::OnAbilitySystemReady()
{
    Super::OnAbilitySystemReady();

    UTestGameAbilitySystemComponent* ASC =
        Cast<UTestGameAbilitySystemComponent>(
            GetObservedAbilitySystem()
        );

    if (!ASC)
    {
        return;
    }

    TestGameAbilitySystemComponent = ASC;

    AActor* PlayerActor = GetObservedActor();

    if (WBP_PlayerHealthBar)
    {
        WBP_PlayerHealthBar->InitializeFromActor(PlayerActor);
    }

    if (WBP_PlayerResourceBar)
    {
        WBP_PlayerResourceBar->InitializeFromActor(PlayerActor);
    }

    if (WBP_BuffBar)
    {
        WBP_BuffBar->InitializeFromActor(PlayerActor);
    }

    AbilityBarChangedHandle =
        ASC->OnAbilityBarChanged.AddUObject(
            this,
            &UPlayerHudWidget::RefreshAbilitySlots
        );

    BuildAbilitySlots();
    RefreshAbilitySlots();
}

void UPlayerHudWidget::UnbindFromAbilitySystem()
{
    if (TestGameAbilitySystemComponent.IsValid() &&
        AbilityBarChangedHandle.IsValid())
    {
        TestGameAbilitySystemComponent
            ->OnAbilityBarChanged
            .Remove(AbilityBarChangedHandle);
    }

    AbilityBarChangedHandle.Reset();
    TestGameAbilitySystemComponent.Reset();

    if (WBP_PlayerHealthBar)
    {
        WBP_PlayerHealthBar->InitializeFromActor(nullptr);
    }

    if (WBP_PlayerResourceBar)
    {
        WBP_PlayerResourceBar->InitializeFromActor(nullptr);
    }

    if (WBP_BuffBar)
    {
        WBP_BuffBar->InitializeFromActor(nullptr);
    }

    Super::UnbindFromAbilitySystem();
}

void UPlayerHudWidget::BuildAbilitySlots()
{
    UTestGameAbilitySystemComponent* ASC =
        TestGameAbilitySystemComponent.Get();

    if (!HP_AbilityBar ||
        !AbilitySlotWidgetClass ||
        !ASC)
    {
        return;
    }

    HP_AbilityBar->ClearChildren();
    AbilitySlotWidgets.Empty();

    static const FName SlotTags[] =
    {
        TEXT("Input.Ability.Slot1"),
        TEXT("Input.Ability.Slot2"),
        TEXT("Input.Ability.Slot3"),
        TEXT("Input.Ability.Slot4"),
        TEXT("Input.Ability.Slot5"),
        TEXT("Input.Ability.Slot6")
    };

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(SlotTags); ++Index)
    {
        const FGameplayTag InputTag =
            FGameplayTag::RequestGameplayTag(SlotTags[Index]);

        UAbilitySlotWidget* SlotWidget =
            CreateWidget<UAbilitySlotWidget>(
                GetOwningPlayer(),
                AbilitySlotWidgetClass
            );

        if (!SlotWidget)
        {
            continue;
        }

        SlotWidget->InitializeSlot(
            ASC,
            InputTag,
            FText::AsNumber(Index + 1)
        );

        UHorizontalBoxSlot* BoxSlot =
            HP_AbilityBar->AddChildToHorizontalBox(SlotWidget);

        // AbilitySlot Padding
        if (BoxSlot)
        {
            BoxSlot->SetPadding(
                FMargin(1.0f, 0.0f, 1.0f, 0.0f)
            );
        }

        AbilitySlotWidgets.Add(SlotWidget);
    }
}

void UPlayerHudWidget::RefreshAbilitySlots()
{
    for (UAbilitySlotWidget* SlotWidget : AbilitySlotWidgets)
    {
        if (SlotWidget)
        {
            SlotWidget->RefreshAbility();
        }
    }
}
