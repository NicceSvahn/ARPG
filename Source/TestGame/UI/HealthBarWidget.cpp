#include "HealthBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHealthBarWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
    const float HealthPercent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;

    if (HealthBar)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SetHealth: Current=%f Max=%f Percent=%f Bar=%s"),
            CurrentHealth,
            MaxHealth,
            HealthPercent,
            *GetNameSafe(HealthBar));

        HealthBar->SetPercent(HealthPercent);
    }
}