#include "PlayerHudWidget.h"

#include "AbilitySlotWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

#include "../Characters/GenericCharacter.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "HealthBarWidget.h"

void UPlayerHudWidget::InitializeHud(
    AGenericCharacter* InCharacter)
{
    if (!InCharacter)
    {
        return;
    }

    if (AbilitySystemComponent &&
        AbilityBarChangedHandle.IsValid())
    {
        AbilitySystemComponent
            ->OnAbilityBarChanged
            .Remove(AbilityBarChangedHandle);
    }

    if (PlayerCharacter &&
        HealthChangedHandle.IsValid())
    {
        PlayerCharacter
            ->OnHealthChanged
            .Remove(HealthChangedHandle);
    }

    PlayerCharacter = InCharacter;

    AbilitySystemComponent =
        PlayerCharacter->GetAbilitySystemComponent();

    if (!AbilitySystemComponent)
    {
        return;
    }

    AbilityBarChangedHandle =
        AbilitySystemComponent
        ->OnAbilityBarChanged
        .AddUObject(
            this,
            &UPlayerHudWidget::RefreshAbilitySlots);

    HealthChangedHandle =
        PlayerCharacter
        ->OnHealthChanged
        .AddUObject(
            this,
            &UPlayerHudWidget::SetHealth);

    BuildAbilitySlots();

    RefreshAbilitySlots();

    SetHealth(
        PlayerCharacter->GetCurrentHealth(),
        PlayerCharacter->GetMaxHealth()
    );
}

void UPlayerHudWidget::BuildAbilitySlots()
{
    if (!HP_AbilityBar ||
        !AbilitySlotWidgetClass ||
        !AbilitySystemComponent)
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

    for (int32 Index = 0; Index < 6; ++Index)
    {
        const FGameplayTag InputTag =
            FGameplayTag::RequestGameplayTag(
                SlotTags[Index]);

        UAbilitySlotWidget* SlotWidget =
            CreateWidget<UAbilitySlotWidget>(
                GetOwningPlayer(),
                AbilitySlotWidgetClass);

        if (!SlotWidget)
        {
            continue;
        }

        SlotWidget->InitializeSlot(
            AbilitySystemComponent,
            InputTag,
            FText::AsNumber(Index + 1));

        UHorizontalBoxSlot* BoxSlot =
            HP_AbilityBar
            ->AddChildToHorizontalBox(
                SlotWidget);

        if (BoxSlot)
        {
            BoxSlot->SetPadding(
                FMargin(
                    3.0f,
                    0.0f,
                    3.0f,
                    0.0f));
        }

        AbilitySlotWidgets.Add(
            SlotWidget);
    }
}

void UPlayerHudWidget::RefreshAbilitySlots()
{
    for (UAbilitySlotWidget* SlotWidget :
        AbilitySlotWidgets)
    {
        if (SlotWidget)
        {
            SlotWidget->RefreshAbility();
        }
    }
}


void UPlayerHudWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
    if (!IsValid(WBP_PlayerHealthBar))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s: HealthBarWidget is not bound."),
            *GetNameSafe(this));

        return;
    }

    WBP_PlayerHealthBar->SetHealth(CurrentHealth, MaxHealth);
}


void UPlayerHudWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetHealth(100.f, 100.f);
}

void UPlayerHudWidget::NativeDestruct()
{
    if (AbilitySystemComponent &&
        AbilityBarChangedHandle.IsValid())
    {
        AbilitySystemComponent
            ->OnAbilityBarChanged
            .Remove(AbilityBarChangedHandle);
    }

    if (PlayerCharacter &&
        HealthChangedHandle.IsValid())
    {
        PlayerCharacter
            ->OnHealthChanged
            .Remove(HealthChangedHandle);
    }

    Super::NativeDestruct();
}