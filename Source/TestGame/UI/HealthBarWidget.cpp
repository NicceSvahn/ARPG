#include "HealthBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"

#include "../AbilitySystem/Attributes/HealthAttributeSet.h"

void UHealthBarWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
    if (!HealthBar)
    {
        return;
    }

    const float HealthPercent =
        MaxHealth > 0.0f
            ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
            : 0.0f;

    HealthBar->SetPercent(HealthPercent);
}

void UHealthBarWidget::OnAbilitySystemReady()
{
    Super::OnAbilitySystemReady();

    UAbilitySystemComponent* ASC = GetObservedAbilitySystem();
    if (!ASC)
    {
        return;
    }

    HealthChangedHandle =
        ASC->GetGameplayAttributeValueChangeDelegate(
            UHealthAttributeSet::GetHealthAttribute()
        ).AddUObject(
            this,
            &UHealthBarWidget::HandleHealthChanged
        );

    MaxHealthChangedHandle =
        ASC->GetGameplayAttributeValueChangeDelegate(
            UHealthAttributeSet::GetMaxHealthAttribute()
        ).AddUObject(
            this,
            &UHealthBarWidget::HandleMaxHealthChanged
        );

    RefreshHealth();
}

void UHealthBarWidget::UnbindFromAbilitySystem()
{
    UAbilitySystemComponent* ASC = GetObservedAbilitySystem();

    if (ASC)
    {
        if (HealthChangedHandle.IsValid())
        {
            ASC->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetHealthAttribute()
            ).Remove(HealthChangedHandle);
        }

        if (MaxHealthChangedHandle.IsValid())
        {
            ASC->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetMaxHealthAttribute()
            ).Remove(MaxHealthChangedHandle);
        }
    }

    HealthChangedHandle.Reset();
    MaxHealthChangedHandle.Reset();

    Super::UnbindFromAbilitySystem();
}

void UHealthBarWidget::RefreshHealth()
{
    UAbilitySystemComponent* ASC = GetObservedAbilitySystem();
    if (!ASC)
    {
        return;
    }

    const float CurrentHealth =
        ASC->GetNumericAttribute(
            UHealthAttributeSet::GetHealthAttribute()
        );

    const float MaxHealth =
        ASC->GetNumericAttribute(
            UHealthAttributeSet::GetMaxHealthAttribute()
        );

    SetHealth(CurrentHealth, MaxHealth);
}

void UHealthBarWidget::HandleHealthChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshHealth();
}

void UHealthBarWidget::HandleMaxHealthChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshHealth();
}
