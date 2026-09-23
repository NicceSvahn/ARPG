#include "PlayerHudWidget.h"

#include "AbilitySlotWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"

#include "HealthBarWidget.h"
#include "../Characters/GenericCharacter.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../UI/ResourceBarWidget.h"
#include "../AbilitySystem/Attributes/ResourceAttributeSet.h"

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

    if (AbilitySystemComponent &&
        HealthChangedHandle.IsValid())
    {
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetHealthAttribute()
            )
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

    BuildAbilitySlots();

    RefreshAbilitySlots();

    SetHealth(
        PlayerCharacter->GetCurrentHealth(),
        PlayerCharacter->GetMaxHealth()
    );

    HealthChangedHandle =
        AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(
            UHealthAttributeSet::GetHealthAttribute()
        )
        .AddUObject(
            this,
            &UPlayerHudWidget::HandleHealthChanged
        );

    ResourceChangedHandle =
        AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(
            UResourceAttributeSet::GetResourceAttribute()
        )
        .AddUObject(
            this,
            &UPlayerHudWidget::HandleResourceChanged
        );

    MaxResourceChangedHandle =
        AbilitySystemComponent
        ->GetGameplayAttributeValueChangeDelegate(
            UResourceAttributeSet::GetMaxResourceAttribute()
        )
        .AddUObject(
            this,
            &UPlayerHudWidget::HandleMaxResourceChanged
        );

    RefreshResourceBar();
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
        return;
    }

    WBP_PlayerHealthBar->SetHealth(CurrentHealth, MaxHealth);
}


void UPlayerHudWidget::NativeConstruct()
{
    ;
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

    if (AbilitySystemComponent &&
        HealthChangedHandle.IsValid())
    {
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetHealthAttribute()
            )
            .Remove(HealthChangedHandle);
    }

    if (AbilitySystemComponent)
    {
        if (ResourceChangedHandle.IsValid())
        {
            AbilitySystemComponent
                ->GetGameplayAttributeValueChangeDelegate(
                    UResourceAttributeSet::GetResourceAttribute()
                )
                .Remove(ResourceChangedHandle);
        }

        if (MaxResourceChangedHandle.IsValid())
        {
            AbilitySystemComponent
                ->GetGameplayAttributeValueChangeDelegate(
                    UResourceAttributeSet::GetMaxResourceAttribute()
                )
                .Remove(MaxResourceChangedHandle);
        }
    }
}

void UPlayerHudWidget::HandleResourceChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshResourceBar();
}

void UPlayerHudWidget::HandleMaxResourceChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshResourceBar();
}

void UPlayerHudWidget::RefreshResourceBar()
{
    if (!AbilitySystemComponent ||
        !WBP_PlayerResourceBar)
    {
        return;
    }

    const float CurrentResource =
        AbilitySystemComponent->GetNumericAttribute(
            UResourceAttributeSet::GetResourceAttribute()
        );

    const float MaxResource =
        AbilitySystemComponent->GetNumericAttribute(
            UResourceAttributeSet::GetMaxResourceAttribute()
        );

    WBP_PlayerResourceBar->SetResource(
        CurrentResource,
        MaxResource
    );
}

void UPlayerHudWidget::HandleHealthChanged(
    const FOnAttributeChangeData& Data)
{
    if (!PlayerCharacter)
    {
        return;
    }

    SetHealth(
        Data.NewValue,
        PlayerCharacter->GetMaxHealth()
    );
}