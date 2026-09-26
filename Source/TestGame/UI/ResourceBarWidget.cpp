#include "ResourceBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"

#include "../AbilitySystem/Attributes/ResourceAttributeSet.h"

void UResourceBarWidget::OnAbilitySystemReady()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        return;
    }

    ResourceChangedHandle =
        ASC
        ->GetGameplayAttributeValueChangeDelegate(
            UResourceAttributeSet::GetResourceAttribute()
        )
        .AddUObject(
            this,
            &UResourceBarWidget::HandleResourceChanged
        );

    MaxResourceChangedHandle =
        ASC
        ->GetGameplayAttributeValueChangeDelegate(
            UResourceAttributeSet::GetMaxResourceAttribute()
        )
        .AddUObject(
            this,
            &UResourceBarWidget::HandleMaxResourceChanged
        );

    // Important:
    // Immediately fetch the current GAS values so the bar
    // is correct before the first resource change happens.
    RefreshResource();
}

void UResourceBarWidget::UnbindFromAbilitySystem()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        ResourceChangedHandle.Reset();
        MaxResourceChangedHandle.Reset();
        return;
    }

    if (ResourceChangedHandle.IsValid())
    {
        ASC
            ->GetGameplayAttributeValueChangeDelegate(
                UResourceAttributeSet::GetResourceAttribute()
            )
            .Remove(ResourceChangedHandle);

        ResourceChangedHandle.Reset();
    }

    if (MaxResourceChangedHandle.IsValid())
    {
        ASC
            ->GetGameplayAttributeValueChangeDelegate(
                UResourceAttributeSet::GetMaxResourceAttribute()
            )
            .Remove(MaxResourceChangedHandle);

        MaxResourceChangedHandle.Reset();
    }
}

void UResourceBarWidget::RefreshResource()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        return;
    }

    const float CurrentResource =
        ASC->GetNumericAttribute(
            UResourceAttributeSet::GetResourceAttribute()
        );

    const float MaxResource =
        ASC->GetNumericAttribute(
            UResourceAttributeSet::GetMaxResourceAttribute()
        );

    SetResource(
        CurrentResource,
        MaxResource
    );
}

void UResourceBarWidget::HandleResourceChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshResource();
}

void UResourceBarWidget::HandleMaxResourceChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshResource();
}

void UResourceBarWidget::SetResource(
    float CurrentResource,
    float MaxResource)
{
    if (!ResourceBar)
    {
        return;
    }

    const float Percent =
        MaxResource > 0.0f
        ? FMath::Clamp(
            CurrentResource / MaxResource,
            0.0f,
            1.0f
        )
        : 0.0f;

    ResourceBar->SetPercent(Percent);
}